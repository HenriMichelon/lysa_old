/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.memory;

import std;
import vireo;
import lysa.types;

export namespace lysa {

    struct MemoryBlock {
        uint32 instanceIndex{0};
        std::size_t offset{0};
        std::size_t size{0};

        friend bool operator==(const MemoryBlock&first, const MemoryBlock&second) {
            return first.offset == second.offset && first.size == second.size;
        }
    };

    class MemoryArray {
    public:
        MemoryBlock alloc(std::size_t instanceCount);

        void free(const MemoryBlock& bloc);

        virtual void write(const MemoryBlock& destination, const void* source) = 0;

        void copyTo(const vireo::CommandList& commandList, const MemoryArray& destination);

        auto getBuffer() const { return buffer; }

        virtual void cleanup();

        virtual ~MemoryArray();
        MemoryArray(MemoryArray&) = delete;
        MemoryArray& operator=(MemoryArray&) = delete;

    protected:
        const std::string name;
        const std::size_t instanceSize;
        std::shared_ptr<vireo::Buffer> buffer;
        std::list<MemoryBlock> freeBlocs;
        std::mutex mutex;

        MemoryArray(
            const vireo::Vireo& vireo,
            std::size_t instanceSize,
            std::size_t instanceCount,
            vireo::BufferType bufferType,
            const std::string& name);
    };

    class DeviceMemoryArray : public MemoryArray {
    public:
        DeviceMemoryArray(
            const vireo::Vireo& vireo,
            std::size_t instanceSize,
            std::size_t instanceCount,
            std::size_t stagingInstanceCount,
            vireo::BufferType,
            const std::string& name);

        void write(const MemoryBlock& destination, const void* source) override;

        void flush(const vireo::CommandList& commandList);

        void postBarrier(const vireo::CommandList& commandList) const;

        void cleanup() override;

        ~DeviceMemoryArray() override;

    private:
        std::shared_ptr<vireo::Buffer> stagingBuffer;
        std::size_t stagingBufferCurrentOffset{0};
        std::vector<vireo::BufferCopyRegion> pendingWrites;
    };

    class HostVisibleMemoryArray : public MemoryArray {
    public:
        HostVisibleMemoryArray(
            const vireo::Vireo& vireo,
            std::size_t instanceSize,
            std::size_t instanceCount,
            vireo::BufferType,
            const std::string& name);

        void write(const MemoryBlock& destination, const void* source) override;
    };

}
