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
        const size_t stagingInstanceCount,
        const vireo::BufferType bufferType,
        const std::wstring& name) :
        name{name},
        instanceSize{instanceSize},
        buffer{vireo.createBuffer(bufferType, instanceSize * instanceCount, 1, name)},
        stagingBuffer{vireo.createBuffer(vireo::BufferType::BUFFER_UPLOAD, instanceSize * stagingInstanceCount, 1, L"Staging " + name)} {
        freeBlocs.push_back({0, 0, instanceSize * instanceCount});
        stagingBuffer->map();
    }

    MemoryArray::~MemoryArray() {
        cleanup();
    }

    void MemoryArray::cleanup() {
        buffer.reset();
        stagingBuffer.reset();
    }

    void MemoryArray::write(const MemoryBlock& destination, const void* source) {
        stagingBuffer->write(source, destination.size, stagingBufferCurrentOffset);
        pendingWrites.push_back({
            stagingBufferCurrentOffset,
            destination.offset,
            destination.size,
        });
        stagingBufferCurrentOffset += destination.size;
    }

    void MemoryArray::flush(vireo::CommandList& commandList) {
        commandList.copy(stagingBuffer, buffer, pendingWrites);
        stagingBufferCurrentOffset = 0;
        pendingWrites.clear();
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

}
