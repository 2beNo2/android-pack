# android-pack
  - 学习Android的加壳技术
## AndroidPack1: 
  - Android的第一代壳
  - 使用`DexClassLoader`加载dex文件
  - 替换掉当前虚拟机的`ClassLoader`，因其保存了对应的`DexFile`
  - 缺点：
    - 因`DexClassLoader`的构造函数中需指定dex文件的路径
    - 故对文件I/O进行监控，便能获取到目标dex文件
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

## AndroidPack3: 
  - Android的第三代壳
  - 缓解了二代壳中的内存dump问题，即破坏dex文件在内存中的完整性
  - 大致实现思路：
    - 抽取目标函数的`codeItem`，将其额外保存
    - 禁用art虚拟机的`dex2oat`优化流程，主要hook`execve`方法实现
    - hook虚拟机的`LoadMethod`方法，在函数调用时将`codeItem`修复
  - 缺点：
    - 仍然无法完全解决内存dump的问题
    - 因dex字节码最终还是要交给art虚拟机解析执行，修改系统源码即可还原dex字节码
  - 解决办法：
    - 自实现art虚拟机，强迫逆向者逆向native层代码
