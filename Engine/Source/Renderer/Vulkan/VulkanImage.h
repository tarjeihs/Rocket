#pragma once

struct VkImage_T;
struct VkImageView_T;
struct VmaAllocation_T;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;
typedef struct VmaAllocation_T* VmaAllocation;

struct VkExtent2D;
struct VkExtent3D;
enum VkImageLayout;
enum VkFormat;

class PVulkanRHI;
class PVulkanCommandBuffer;

class PVulkanImage
{
public:
	void Init(VkExtent2D Extent, VkFormat Format);
	void Reset();

	void ApplyImage(VkImage Image);
	void ApplyImageView(VkImageView ImageView);

	void CreateImage(VkImageUsageFlags ImageUsageFlags);
	void CreateImageView(VkImageAspectFlags ImageViewAspectFlags);
	
	void DestroyImage();
	void DestroyImageView();

	void TransitionImageLayout(PVulkanCommandBuffer* CommandBuffer, VkImageLayout CurrentLayout, VkImageLayout NewLayout);
	void CopyImageRegion(PVulkanCommandBuffer* CommandBuffer, VkImage Dest, VkExtent2D SrcSize, VkExtent2D DstSize);

	VkImage GetVkImage() const;
	VkImageView GetVkImageView() const;
	VkExtent2D GetImageExtent2D() const;
	VkFormat GetVkFormat() const;

	VkImage ImageHandle;
	VkImageView ImageViewHandle;
	VmaAllocation MemoryAllocation;
	VkFormat ImageFormat;
	VkExtent2D ImageExtent;
	VkExtent3D ImageExtent3D;
};