# Contributing

## Add a Note

1. 一个文件只讲清一个主题。
2. 先解释问题和适用边界，再给代码。
3. 代码示例必须说明运行环境、线程安全与中断上下文限制。
4. 外部资料使用链接和自己的总结，不复制大段原文。

## Add C Code

- 使用 C11，固定宽度整数类型来自 `<stdint.h>`。
- 公共接口检查空指针、长度和容量。
- 不在通用模块中依赖特定芯片 HAL。
- 可在主机运行的逻辑必须补充测试。
- 提交前执行：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Commit Style

使用 `feat:`、`fix:`、`docs:`、`test:`、`refactor:` 等前缀。标题使用祈使语气并保持简洁。

