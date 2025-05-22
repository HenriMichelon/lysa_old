/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.memory;

import lysa.application;
import lysa.global;

namespace lysa {

    MemoryArray::MemoryArray(
        const vireo::Vireo& vireo,
        const size_t instanceSize,
        const size_t instanceCount,
        const vireo::BufferType bufferType,
        const std::wstring& name) :
        name{name},
        instanceSize{instanceSize},
        buffer{vireo.createBuffer(bufferType, instanceSize * instanceCount, 1, name)} {
        freeBlocs.push_back({0, 0, instanceSize * instanceCount});
    }

    MemoryArray::~MemoryArray() {
        MemoryArray::cleanup();
    }

    void MemoryArray::cleanup() {
        buffer.reset();
    }

    MemoryBlock MemoryArray::alloc(const size_t instanceCount) {
        const auto size = instanceSize * instanceCount;
        for (MemoryBlock& bloc : freeBlocs) {
            if (bloc.size >= size) {
                const MemoryBlock result{
                    static_cast<uint32>(bloc.offset / instanceSize),
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
        throw Exception{"Out of memory for array " + std::to_string(name)};
    }

    void MemoryArray::free(const MemoryBlock& bloc) {
        freeBlocs.push_back(bloc);
    }

    void MemoryArray::copyTo(const vireo::CommandList& commandList, const MemoryArray& destination) const {
        commandList.copy(buffer, destination.buffer);
    }

    DeviceMemoryArray::DeviceMemoryArray(
        const vireo::Vireo& vireo,
        const size_t instanceSize,
        const size_t instanceCount,
        const size_t stagingInstanceCount,
        const vireo::BufferType bufferType,
        const std::wstring& name) :
        MemoryArray{vireo, instanceSize, instanceCount, bufferType, name},
        stagingBuffer{vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, instanceSize * stagingInstanceCount, 1, L"Staging " + name)} {
        assert([&]{ return bufferType == vireo::BufferType::VERTEX ||
            bufferType == vireo::BufferType::INDEX ||
            bufferType == vireo::BufferType::INDIRECT ||
            bufferType == vireo::BufferType::DEVICE_STORAGE ||
            bufferType == vireo::BufferType::READWRITE_STORAGE;}, "Invalid buffer type for device memory array");
        stagingBuffer->map();
    }

    void DeviceMemoryArray::write(const MemoryBlock& destination, const void* source) {
        stagingBuffer->write(source, destination.size, stagingBufferCurrentOffset);
        pendingWrites.push_back({
            stagingBufferCurrentOffset,
            destination.offset,
            destination.size,
        });
        stagingBufferCurrentOffset += destination.size;
    }

    void DeviceMemoryArray::flush(const vireo::CommandList& commandList) {
        commandList.copy(stagingBuffer, buffer, pendingWrites);
        stagingBufferCurrentOffset = 0;
        pendingWrites.clear();
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
        const std::wstring& name) :
        MemoryArray{vireo, instanceSize, instanceCount, bufferType, name} {
        assert([&]{ return bufferType == vireo::BufferType::UNIFORM ||
            bufferType == vireo::BufferType::STORAGE ||
            bufferType == vireo::BufferType::BUFFER_UPLOAD ||
            bufferType == vireo::BufferType::IMAGE_DOWNLOAD ||
            bufferType == vireo::BufferType::IMAGE_UPLOAD;}, "Invalid buffer type for host visible memory array");
        buffer->map();
    }

    void HostVisibleMemoryArray::write(const MemoryBlock& destination, const void* source) {
        buffer->write(source, destination.size, destination.offset);
    }

 }
