# Vulcant API Technical Manual

---

## Overview

The **Vulcant API** provides an abstract object-oriented C++ interface for Vulkan graphics and compute programming. It encapsulates hardware resource management, pipeline configuration, compute and graphic command recording, synchronization barriers, and image operations.

---

## Common Enumerations

### `VulcantBufferType`
Specifies the binding type and usage intent of a buffer.

| Value | Description |
| :--- | :--- |
| `Storage` | High-capacity read/write buffer used primarily in compute workflows. |
| `Uniform` | Read-only buffer for constant/uniform shader parameters. |
| `Vertex` | Buffer containing vertex attribute data for graphic pipelines. |
| `Index` | Buffer containing draw indices. |

---

### `VulcantImageFormat`
Specifies image color channels and memory depth, mapped to underlying Vulkan formats.

| Value | Native Vulkan Equivalent | Description |
| :--- | :--- | :--- |
| `R32G32B32A32_SFLOAT` | `VK_FORMAT_R32G32B32A32_SFLOAT` | 128-bit RGBA 32-bit floating-point format. |
| `R16G16B16A16_SFLOAT` | `VK_FORMAT_R16G16B16A16_SFLOAT` | 64-bit RGBA 16-bit floating-point format. |
| `R8G8B8A8_UNORM` | `VK_FORMAT_R8G8B8A8_UNORM` | 32-bit RGBA normalized unsigned integer format. |
| `R32_SFLOAT` | `VK_FORMAT_R32_SFLOAT` | 32-bit single-channel floating-point format. |
| `D32_SFLOAT` | `VK_FORMAT_D32_SFLOAT` | 32-bit single-channel floating-point depth format. |
| `Unkown` | — | Uninitialized or unsupported image format. |

---

### `VulcantImageUsage`
Defines operational capabilities and memory access patterns for images.

| Value | Description |
| :--- | :--- |
| `WriteAndRead` | Storage image layout allowing concurrent read/write access from shaders. |
| `SampleOnly` | Shader read-only format for combined texture sampling. |
| `TransferSource` | Image configured as transfer source for CPU readbacks and screenshots. |
| `TransferDest` | Image configured as transfer destination for CPU texture uploads. |

---

## Core Interfaces

### 1. `VulcantDevice`
Primary factory class for allocating Vulkan GPU resources, creating pipelines, and spawning execution context wrappers.

#### Member Functions

* `std::unique_ptr<VulcantBuffer> createBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly = false)`  
  Allocates a general-purpose data buffer.
  * `numberOfElements`: Total number of elements stored.
  * `elementSize`: Size per element in bytes.
  * `gpuOnly`: If `true`, allocates in high-speed VRAM inaccessible directly by CPU.

* `std::unique_ptr<VulcantBuffer> createUniform(size_t numberOfElements, size_t elementSize)`  
  Allocates a uniform buffer object (UBO) for pass parameters and constant data.

* `std::unique_ptr<VulcantBuffer> createVertexBuffer(size_t numberOfElements, size_t elementSize, bool gpuOnly = false)`  
  Allocates a vertex buffer object (VBO).

* `std::unique_ptr<VulcantShader> createShader(const std::string& shader)`  
  Compiles and loads a shader module from source code string or file path.

* `std::unique_ptr<VulcantShader> createShader(const std::vector<uint32_t>& spirv)`  
  Loads a compiled SPIR-V binary array into a shader module.

* `std::unique_ptr<VulcantImage> createImage(uint32_t width, uint32_t height, uint32_t depth = 1, VulcantImageFormat format = VulcantImageFormat::R32G32B32A32_SFLOAT)`  
  Allocates a 2D or 3D image buffer using the specified format.

* `std::unique_ptr<VulcantComputeCommand> createComputeCommand()`  
  Instantiates an independent command buffer context for compute dispatches.

* `std::unique_ptr<VulcantGraphicCommand> createGraphicCommand()`  
  Instantiates an independent command buffer context for graphic draw commands.

* `std::unique_ptr<VulcantGraphicPipeline> createVulcanGraphicPipeline(const std::vector<VulcantShader*>& shader, const std::vector<VulcantImage*>& color, VulcantImage* depth = nullptr, VulcantImage* stencil = nullptr)`  
  Constructs a graphic pipeline object bound to color render targets and optional depth/stencil attachments.

* `std::unique_ptr<VulcantSet> createSet(const std::vector<std::vector<std::shared_ptr<VulcantResource>>>& buffer, VulcantShader& shader)`  
  Binds resource layout sets to target shader reflection requirements.

* `std::unique_ptr<VulcantWindow> createWindow(const glm::ivec2& resolution, const std::string& title)`  
  Creates a native display window context.

* `std::unique_ptr<VulcantUi> createUi(VulcantWindow& window)`  
  Creates an overlay UI context bound to the specified window.

* `VulcantComputeCommand& getDefaultCommand()`  
  Returns a reference to the global default compute command queue.

---

### 2. `VulcantBuffer`
Interface representing memory-backed data stores on CPU/GPU.

#### Member Functions

* `std::unique_ptr<VulcantResource> asResource() const`  
  Wraps the buffer as a polymorphic resource binding object usable in descriptor set generation.

* `size_t getNumberOfElements() const`  
  Returns total allocated element count.

* `size_t getElementSize() const`  
  Returns size per element in bytes.

* `void uploadToGPU(const void* data, size_t elementCount, size_t elementOffset = 0)`  
  Copies data from host CPU pointer to GPU buffer memory.
  * `data`: Pointer to source host data.
  * `elementCount`: Number of elements to transfer.
  * `elementOffset`: Element offset index within destination buffer.

* `void downloadFromGPU(void* outData, size_t elementCount, size_t elementOffset = 0)`  
  Reads data back from GPU memory into host CPU memory target.

---

### 3. `VulcantImage`
Interface representing 2D/3D textures and render targets.

#### Member Functions

* `glm::ivec2 getResolution() const`  
  Returns two-dimensional image extents `(width, height)`.

* `std::unique_ptr<VulcantResource> asResource() const`  
  Wraps the image into a resource handle for descriptor sets.

* `void saveRenderedImage(const std::string& path)`  
  Reads image back from VRAM and writes contents directly to disk.

* `void setUsage(const Vulcant::VulcantImageUsage& usage)`  
  Updates intended resource access flags.

---

### 4. `VulcantComputeCommand`
Command buffer management context for compute dispatches, buffer copies, and image transfers.

#### Member Functions

* `void startRecord()` / `void endRecord()`  
  Begins and finishes compute command buffer recording state.

* `void add(const glm::ivec3& groupCount, VulcantSet& setInput, VulcantShader& pipelineInput)`  
  Enqueues a compute dispatch workload.
  * `groupCount`: 3D grid dimensions `(WorkGroupsX, WorkGroupsY, WorkGroupsZ)`.
  * `setInput`: Descriptor resources passed to invocation.
  * `pipelineInput`: Compute shader stage to execute.

* `void addBarrier(VulcantBuffer&)`  
  Inserts an execution/memory pipeline barrier for buffer synchronization.

* `void addBarrier(VulcantImage&, const VulcantResourceLayout& dest = VulcantResourceLayout::General)`  
  Inserts a pipeline image layout transition barrier.

* `void addCopyBuffer(VulcantBuffer& source, VulcantBuffer& dest, size_t elementCount, size_t sourceOffset = 0, size_t destOffset = 0)`  
  Enqueues GPU-side buffer-to-buffer memory copy operations.

* `void uploadImageToGPU(VulcantImage& destImage, const void* cpuData, glm::uvec3 extent, glm::uvec3 offset = glm::uvec3(0, 0, 0))`  
  Copies raw pixel data from CPU host memory into GPU image space.

* `void downloadImageFromGPU(VulcantImage& srcImage, void* outData, glm::uvec3 extent, glm::uvec3 offset)`  
  Copies GPU image contents to CPU output memory array.

* `void runSync()`  
  Submits recorded commands and blocks calling host thread until execution completes.

* `void runAsync()`  
  Submits recorded commands asynchronously to the compute queue.

* `void wait()`  
  Blocks CPU thread until asynchronously submitted commands complete execution.

---

### 5. `VulcantGraphicCommand`
Command context for managing graphics render passes, pipeline state, and draw execution.

#### Member Functions

* `void startRecord()` / `void endRecord()`  
  Opens and closes graphics command recording context.

* `void beginRendering(VulcantGraphicPipeline& pipeline)`  
  Binds specified graphic pipeline state and initiates a render pass instance.

* `void endRendering()`  
  Concludes active render pass instance.

* `void setViewportAndScissor(glm::uvec2 extent)`  
  Sets dynamic viewport resolution and scissor rectangle dimensions.

* `void draw(uint32_t vertexCount, VulcantSet* set = nullptr, VulcantBuffer* vertexBuffer = nullptr)`  
  Issues non-indexed draw command.
  * `vertexCount`: Total vertices to render.
  * `set`: Optional descriptor set binding.
  * `vertexBuffer`: Optional input stream containing vertex attributes.

* `void addBarrier(VulcantImage& inputImg, const VulcantResourceLayout& dest)`  
  Inserts memory barrier to transition layout state for images.

* `void addBarrier(VulcantBuffer& buffer)`  
  Inserts execution pipeline barrier for synchronization across draw passes.

* `void runSync()` / `void runAsync()` / `void wait()`  
  Provides execution synchronization matching `VulcantComputeCommand` execution models.

---

### 6. `VulcantGraphicPipeline`
Opaque handle interface encapsulating Vulkan rasterization state, fixed-function stages, and shader binding states. Constructed directly via `VulcantDevice::createVulcanGraphicPipeline`.
