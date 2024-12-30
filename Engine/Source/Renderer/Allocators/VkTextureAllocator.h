struct FVkTextureAllocator
{
    struct FInstanceMetadata
    {
        FInstanceMetadata() = default;
    };

    struct FAllocatorData
    {
        SizeType CurrentTextureOffset = 0;
        SizeType StagingTextureOffset = 0;
    };

    TArray<TOptional<FVkTextureAllocator::FInstanceMetadata>> Metadata;

    void Initialize()
    {
        Metadata.Resize(1024, FVkTextureAllocator::FInstanceMetadata());
    }
};