/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.memory;

import lysa.application;
import lysa.exception;
import lysa.log;

namespace lysa {

    MemoryArray::MemoryArray(
        const vireo::Vireo& vireo,
        const size_t instanceSize,
        const size_t instanceCount,
        const vireo::BufferType bufferType,
        const std::string& name) :
        name{name},
        instanceSize{instanceSize} {
        if (bufferType == vireo::BufferType::VERTEX || bufferType == vireo::BufferType::INDEX) {
            buffer = vireo.createBuffer(bufferType, instanceSize, instanceCount, name);
        } else {
            buffer = vireo.createBuffer(bufferType, instanceSize * instanceCount, 1, name);
        }
        freeBlocs.push_back({0, 0, instanceSize * instanceCount});
    }

    MemoryArray::~MemoryArray() {
        MemoryArray::cleanup();
    }

    void MemoryArray::cleanup() {
        buffer.reset();
    }

    MemoryBlock MemoryArray::alloc(const size_t instanceCount) {
        auto lock = std::lock_guard{mutex};
        const auto size = instanceSize * instanceCount;
        for (MemoryBlock& bloc : freeBlocs) {
            if (bloc.size >= size) {
                const MemoryBlock result{
                    (bloc.offset / instanceSize),
                    bloc.offset,
                    size};
                if (bloc.size == size) {
                    freeBlocs.remove(bloc);
                } else {
                    bloc.offset += size;
                    bloc.size -= size;
                }
                return result;
            }
        }
        throw Exception{"Out of memory for array " + name};
    }

    void MemoryArray::free(const MemoryBlock& bloc) {
        auto lock = std::lock_guard{mutex};
        freeBlocs.push_back(bloc);
    }

    void MemoryArray::copyTo(const vireo::CommandList& commandList, const MemoryArray& destination) {
        auto lock = std::lock_guard{mutex};
        commandList.copy(buffer, destination.buffer);
    }

    DeviceMemoryArray::DeviceMemoryArray(
        const vireo::Vireo& vireo,
        const size_t instanceSize,
        const size_t instanceCount,
        const size_t stagingInstanceCount,
        const vireo::BufferType bufferType,
        const std::string& name) :
        MemoryArray{vireo, instanceSize, instanceCount, bufferType, name},
        stagingBuffer{vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, instanceSize * stagingInstanceCount, 1, "Staging " + name)} {
        assert([&]{ return bufferType == vireo::BufferType::VERTEX ||
            bufferType == vireo::BufferType::INDEX ||
            bufferType == vireo::BufferType::INDIRECT ||
            bufferType == vireo::BufferType::DEVICE_STORAGE ||
            bufferType == vireo::BufferType::READWRITE_STORAGE;}, "Invalid buffer type for device memory array");
        stagingBuffer->map();
    }

    void DeviceMemoryArray::write(const MemoryBlock& destination, const void* source) {
        auto lock = std::lock_guard{mutex};
        stagingBuffer->write(source, destination.size, stagingBufferCurrentOffset);
        pendingWrites.push_back({
            stagingBufferCurrentOffset,
            destination.offset,
            destination.size,
        });
        stagingBufferCurrentOffset += destination.size;
    }

    void DeviceMemoryArray::flush(const vireo::CommandList& commandList) {
        auto lock = std::lock_guard{mutex};
        if (!pendingWrites.empty()) {
            commandList.copy(stagingBuffer, buffer, pendingWrites);
            pendingWrites.clear();
            stagingBufferCurrentOffset = 0;
        }
    }

    void DeviceMemoryArray::postBarrier(const vireo::CommandList& commandList) const {
        commandList.barrier(
           *buffer,
           vireo::ResourceState::COPY_DST,
           vireo::ResourceState::SHADER_READ);
    }

    void DeviceMemoryArray::cleanup() {
        MemoryArray::cleanup();
        stagingBuffer.reset();
    }

    DeviceMemoryArray::~DeviceMemoryArray() {
        DeviceMemoryArray::cleanup();
    }

    HostVisibleMemoryArray::HostVisibleMemoryArray(
        const vireo::Vireo& vireo,
        const size_t instanceSize,
        const size_t instanceCount,
        const vireo::BufferType bufferType,
        const std::string& name) :
        MemoryArray{vireo, instanceSize, instanceCount, bufferType, name} {
        assert([&]{ return bufferType == vireo::BufferType::UNIFORM ||
            bufferType == vireo::BufferType::STORAGE ||
            bufferType == vireo::BufferType::BUFFER_UPLOAD ||
            bufferType == vireo::BufferType::IMAGE_DOWNLOAD ||
            bufferType == vireo::BufferType::IMAGE_UPLOAD;}, "Invalid buffer type for host visible memory array");
        buffer->map();
    }

    void HostVisibleMemoryArray::write(const MemoryBlock& destination, const void* source) {
        auto lock = std::lock_guard{mutex};
        buffer->write(source, destination.size, destination.offset);
    }

 }
