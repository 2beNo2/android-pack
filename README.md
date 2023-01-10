# android-pack
  - 学习Android的加壳技术
## AndroidPack1: 
  - Android的第一代壳
  - 使用`DexClassLoader`加载dex文件
  - 替换掉当前虚拟机的`ClassLoader`，因其保存了对应的`DexFile`
  - 缺点：
    - 因`DexClassLoader`的构造函数中需指定`dex`文件的路径
    - 故对文件I/O进行监控，便能获取到目标`dex`文件
  - 解决办法：
    - 寻找文件不落地的dex加载方式

## AndroidPack2: 
  - Android的第二代壳
  - 解决了一代壳中的文件落地问题
  - 通过调用`OpenMemory`，将内存中的dex文件加载到虚拟机
  - 缺点：
    - 内存中会保存完整的dex文件
    - 使用内存dump即可脱壳
  - 解决办法：
    - 避免内存dump，破坏dex文件在内存中的完整性


