# NyarukoLuckDraw2

未完成，请勿使用。

第二代抽奖程序，对防止抽奖组织者内定作弊优化，基于比特币区块哈希，支持算法重现历史抽奖结果以验证。

## 基本原理

1. 从 [比特币](https://zh-cn.bitcoin.it/wiki/%E7%AE%80%E4%BB%8B) 网络（本软件基于比特币官方软件 [Bitcoin Core](https://bitcoin.org/zh_CN/download) ）获取当前 [区块](https://bitcoin.org/zh_CN/vocabulary#block) 时间戳和哈希（基于 [区块链](https://bitcoin.org/zh_CN/vocabulary#block-chain) 很难篡改）。区块链信息类似于：

```json
{
  "chain": "main",
  "blocks": 820236,
  "headers": 820236,
  "bestblockhash": "00000000000000000002a3cff5989be09b1ec3328b59e73d97f760f0a3ae39f4",
  "difficulty": 67957790298897.88,
  "time": 1702024931,
  "mediantime": 1702022158
}
```

2. 后端根据这个 SHA256 哈希，使用 djb2 算法生成 64 位整数哈希并和时间戳一起保存。代码演示：

```c
unsigned long hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}
```

3. 前端根据该整数值，自由获取某个最小值到最大值之间的一个数字，即抽奖。该数字是恒定的，即从比特币的哈希值中获得了每个区间的固定数值，因此可以从比特币的历史区块中重现这一切。代码演示：

```c
unsigned long randNum(unsigned long num, unsigned long min, unsigned long max) {
    return num % (max - min + 1) + min;
}
```

## 后端

### 区块链信息记录模块

TODO

### 算法模块

TODO

## 前端

### 网页查询工具

TODO

## 部署

TODO

## 使用

### 抽奖组织者

TODO

### 抽奖参与者

TODO

## LICENSE
- [木兰宽松许可证， 第2版](http://license.coscl.org.cn/MulanPSL2)
- [Mulan Permissive Software License，Version 2 (Mulan PSL v2)](http://license.coscl.org.cn/MulanPSL2)