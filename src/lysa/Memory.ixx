/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.memory;

import vireo;
import lysa.global;

export namespace lysa {

    struct MemoryBloc {
        uint32 instanceIndex{0};
        size_t offset{0};
        size_t size{0};

        friend bool operator==(const MemoryBloc&first, const MemoryBloc&second) {
            return first.offset == second.offset && first.size == second.size;
        }
    };

    class MemoryArray {
    public:
        MemoryArray(
            const vireo::Vireo& vireo,
            size_t instanceSize,
            size_t instanceCount,
            size_t stagingInstanceCount,
            vireo::BufferType,
            const std::wstring& name);

        MemoryBloc alloc(size_t instanceCount);

        void free(const MemoryBloc& bloc);

        void write(const MemoryBloc& destination, const void* source);

        void flush(const std::shared_ptr<vireo::CommandList>& commandList);

        auto getBuffer() const { return buffer; }

    private:
        const std::wstring name;
        const size_t instanceSize;
        const std::shared_ptr<vireo::Buffer> buffer;
        const std::shared_ptr<vireo::Buffer> stagingBuffer;
        size_t stagingBufferCurrentOffset{0};
        std::list<MemoryBloc> freeBlocs;
        std::vector<vireo::BufferCopyRegion> pendingWrites;
    };

}
