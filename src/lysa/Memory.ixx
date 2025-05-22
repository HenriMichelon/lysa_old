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

    struct MemoryBlock {
        uint32 instanceIndex{0};
        size_t offset{0};
        size_t size{0};

        friend bool operator==(const MemoryBlock&first, const MemoryBlock&second) {
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

        MemoryBlock alloc(size_t instanceCount);

        void free(const MemoryBlock& bloc);

        void write(const MemoryBlock& destination, const void* source);

        void flush(vireo::CommandList& commandList);

        auto getBuffer() const { return buffer; }

        void cleanup();

        ~MemoryArray();

    private:
        const std::wstring name;
        const size_t instanceSize;
        std::shared_ptr<vireo::Buffer> buffer;
        std::shared_ptr<vireo::Buffer> stagingBuffer;
        size_t stagingBufferCurrentOffset{0};
        std::list<MemoryBlock> freeBlocs;
        std::vector<vireo::BufferCopyRegion> pendingWrites;
    };

}
