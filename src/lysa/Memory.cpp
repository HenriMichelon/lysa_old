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
        const size_t instanceSize,
        const size_t instanceCount,
        const size_t stagingInstanceCount,
        const vireo::BufferType bufferType,
        const std::wstring& name) :
        name{name},
        instanceSize{instanceSize},
        buffer{Application::getVireo()->createBuffer(bufferType, instanceSize * instanceCount, 1, name)},
        stagingBuffer{Application::getVireo()->createBuffer(vireo::BufferType::BUFFER_UPLOAD, instanceSize * stagingInstanceCount, 1, L"Staging " + name)} {
        freeBlocs.push_back({0, instanceSize * instanceCount});
        stagingBuffer->map();
    }

    void MemoryArray::write(const MemoryBloc& destination, const void* source) {
        stagingBuffer->write(source, destination.size, stagingBufferCurrentOffset);
        pendingWrites.push_back({
            stagingBufferCurrentOffset,
            destination.offset,
            destination.size,
        });
        stagingBufferCurrentOffset += destination.size;
    }

    void MemoryArray::flush(const std::shared_ptr<vireo::CommandList>& commandList) {
        commandList->copy(stagingBuffer, buffer, pendingWrites);
        stagingBufferCurrentOffset = 0;
        pendingWrites.clear();
    }

    MemoryBloc MemoryArray::alloc(const size_t instanceCount) {
        const auto size = instanceSize * instanceCount;
        for (MemoryBloc& bloc : freeBlocs) {
            if (bloc.size >= size) {
                const MemoryBloc result{bloc.offset, size};
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

    void MemoryArray::free(const MemoryBloc& bloc) {
        freeBlocs.push_back(bloc);
    }

}
