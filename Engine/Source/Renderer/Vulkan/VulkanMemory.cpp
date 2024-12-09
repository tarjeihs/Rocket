#include "EnginePCH.h"
#include "VulkanMemory.h"

void PVulkanMemory::Init()
{
//	TArray<FVkDescriptorPoolRatio> PoolRatio = {
//
//		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 65536 },
//		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 65536 },
//	};
//
//	FVkDescriptorPoolCreateInfo DescriptorPoolCreateInfo;
//	DescriptorPoolCreateInfo.PoolRatios = PoolRatio;
//	DescriptorPoolCreateInfo.MaxSetCount = 1;
//	DescriptorPoolCreateInfo.Flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
//
//	TSharedPtr<FVkDescriptorPool> DescriptorPool = MakeShared<FVkDescriptorPool>();
//	DescriptorPool->Initialize(DescriptorPoolCreateInfo);
//
//	FVkDescriptorSetLayoutCreateInfo StorageBufferDescriptorSetLayoutCreateInfo;
//	StorageBufferDescriptorSetLayoutCreateInfo.Descriptors = {
//		{ EVkDescriptorType::Storage, 65536 },
//	};
//
//	StorageBufferDescriptorSetLayout = MakeShared<FVkDescriptorSetLayout>();
//	StorageBufferDescriptorSetLayout->CreateDescriptorSetLayout(StorageBufferDescriptorSetLayoutCreateInfo);
//
//	FVkDescriptorSetCreateInfo DescriptorSetCreateInfo;
//	DescriptorSetCreateInfo.DescriptorPool = DescriptorPool;
//	DescriptorSetCreateInfo.DescriptorSetLayout = StorageBufferDescriptorSetLayout;
//	
//	StorageBufferDescriptorSet = MakeShared<PVulkanDescriptorSet>();
//	StorageBufferDescriptorSet->CreateDescriptorSet(DescriptorSetCreateInfo);
//
//	//DescriptorPoolCache.Insert("G_DP_Shared", DescriptorPool);
//	//DescriptorSetLayoutCache.Insert("G_DSL_StorageBuffer", StorageBufferDescriptorSetLayout);
//	//DescriptorSetCache.Insert("G_DS_StorageBuffer", StorageBufferDescriptorSet);
//
//	
//}
}
void PVulkanMemory::Shutdown()
{
//	TSharedPtr<FVkDescriptorPool> DescriptorPool = *DescriptorPoolCache.Find("G_DP_Shared");
//	DescriptorPool->Destroy();
//	StorageBufferDescriptorSetLayout->DestroyDescriptorSetLayout();
}