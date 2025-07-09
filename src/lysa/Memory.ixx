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
        MemoryBlock alloc(size_t instanceCount);

        void free(const MemoryBlock& bloc);

        virtual void write(const MemoryBlock& destination, const void* source) = 0;

        void copyTo(const vireo::CommandList& commandList, const MemoryArray& destination);

        auto getBuffer() const { return buffer; }

        virtual void cleanup();

        virtual ~MemoryArray();
        MemoryArray(MemoryArray&) = delete;
        MemoryArray& operator=(MemoryArray&) = delete;

    protected:
        const std::wstring name;
        const size_t instanceSize;
        std::shared_ptr<vireo::Buffer> buffer;
        std::list<MemoryBlock> freeBlocs;
        std::mutex mutex;

        MemoryArray(
            const vireo::Vireo& vireo,
            size_t instanceSize,
            size_t instanceCount,
            vireo::BufferType bufferType,
            const std::wstring& name);
    };

    class DeviceMemoryArray : public MemoryArray {
    public:
        DeviceMemoryArray(
            const vireo::Vireo& vireo,
            size_t instanceSize,
            size_t instanceCount,
            size_t stagingInstanceCount,
            vireo::BufferType,
            const std::wstring& name);

        void write(const MemoryBlock& destination, const void* source) override;

        void flush(const vireo::CommandList& commandList);

        void cleanup() override;

        ~DeviceMemoryArray() override;

    private:
        std::shared_ptr<vireo::Buffer> stagingBuffer;
        size_t stagingBufferCurrentOffset{0};
        std::vector<vireo::BufferCopyRegion> pendingWrites;
    };

    class HostVisibleMemoryArray : public MemoryArray {
    public:
        HostVisibleMemoryArray(
            const vireo::Vireo& vireo,
            size_t instanceSize,
            size_t instanceCount,
            vireo::BufferType,
            const std::wstring& name);

        void write(const MemoryBlock& destination, const void* source) override;
    };

}
