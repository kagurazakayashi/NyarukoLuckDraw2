/*
npm i -g typescript
tsc drawtest.ts && node drawtest.js && rm drawtest.js
*/
type FileNameData = {
  year: number;
  month: number;
  day: number;
  hour: number;
  minute: number;
  second: number;
  timestamp: number;
  height: number;
  hash: string;
};
type FileContentsData = {
  chain: string;
  blocks: number;
  headers: number;
  bestblockhash: string;
  difficulty: number;
  time: number;
  mediantime: number;
  verificationprogress: number;
  initialblockdownload: boolean;
  chainwork: string;
  size_on_disk: number;
  pruned: boolean;
  pruneheight: number;
  automatic_pruning: boolean;
  prune_target_size: number;
  warnings: string;
};
type FileData = {
  info: FileNameData;
  data: FileContentsData;
};

class DrawDataParse {
  static splitFileName(fileName): FileNameData {
    const parts = fileName.split("_"); // 日期,時間,時間戳,高度,hash
    const date = parts[0];
    const time = parts[1];
    const data: FileNameData = {
      year: parseInt(date.substring(0, 4)),
      month: parseInt(date.substring(4, 6)),
      day: parseInt(date.substring(6, 8)),
      hour: parseInt(time.substring(0, 2)),
      minute: parseInt(time.substring(2, 4)),
      second: parseInt(time.substring(4, 6)),
      timestamp: parseInt(parts[2]),
      height: parseInt(parts[3]),
      hash: parts[4].split(".")[0],
    };
    return data;
  }

  static splitFileContents(fileContents): FileContentsData {
    const data: FileContentsData = JSON.parse(fileContents);
    return data;
  }

  static splitFileData(fileName, fileContents): FileData {
    const fileNameData = this.splitFileName(fileName);
    const fileContentsData = this.splitFileContents(fileContents);
    const data: FileData = {
      info: fileNameData,
      data: fileContentsData,
    };
    return data;
  }
}

class Draw {
  hash: string = "";
  numHash: number = 0;

  constructor(data: FileNameData) {
    this.hash = data.hash;
    this.numHash = this.djb2();
  }

  djb2(str = this.hash): number {
    if (str.length == 0) {
      return -1;
    }
    let hash = 5381;
    for (let i = 0; i < str.length; i++) {
      hash = (hash * 33) ^ str.charCodeAt(i);
    }
    return hash >>> 0;
  }

  sum16t10(str = this.hash): number {
    let sum = 0;
    if (str.length == 0) {
      return sum;
    }
    for (let i = 0; i < str.length; i++) {
      const char: string = str.charAt(i);
      const num: number = parseInt(char, 16);
      sum += num;
    }
    return sum;
  }

  randNum(min: number, max: number, numHash = this.numHash): number {
    const zone: number = max - min + 1;
    if (
      min < 0 ||
      zone < 0 ||
      numHash < 0 ||
      numHash > 4294967295 ||
      zone > numHash
    ) {
      return -1;
    }
    return (numHash % (max - min + 1)) + min;
  }
}

class DrawTest {
  draw: Draw;
  constructor(draw: Draw) {
    this.draw = draw;
  }

  testNumhash(testCount = 100) {
    const testResultPrint: string[] = [];
    draw.numHash = draw.djb2();
    const one: number = draw.numHash;
    console.log(
      "测试 djb2 : 转换为",
      draw.numHash,
      "作为种子, ",
      testCount,
      "次的结果应该一致:"
    );
    for (let i = 0; i < testCount; i++) {
      draw.numHash = draw.djb2();
      if (draw.numHash === one) {
        testResultPrint.push(".");
      } else {
        testResultPrint.push("X");
        console.warn("不一致的结果: ", draw.numHash);
      }
    }
    console.log(testResultPrint.join(""));
  }

  testRandNum(testCount = 100) {
    const min: number = Math.floor(Math.random() * 100);
    const max: number = min + Math.floor(Math.random() * 100);
    const testResultPrint: string[] = [];
    const one: number = draw.randNum(min, max);
    console.log(
      "测试 randNum : 从",
      min,
      "到",
      max,
      "中抽 1 得到",
      one,
      ",",
      testCount,
      "次的结果应该一致:"
    );
    for (let i = 0; i < testCount; i++) {
      const r = draw.randNum(min, max);
      if (r === one) {
        testResultPrint.push(".");
      } else {
        testResultPrint.push("X");
        console.warn("不一致的结果: ", r);
      }
    }
    console.log(testResultPrint.join(""));
  }

  randomHash(hexLen = 64): string {
    const hexRandStr: string[] = new Array(64);
    for (let i = 0; i < hexLen; i++) {
      const randChar: string = Math.floor(Math.random() * 16).toString(16);
      hexRandStr[i] = randChar;
    }
    return hexRandStr.join("");
  }

  testAll(testCount = 10) {
    for (let i = 0; i < testCount; i++) {
      const hexRandStr: string = this.randomHash();
      console.log("\n》第", i + 1, "次测试,", "随机的 64 位 hex: ", hexRandStr);
      draw.hash = hexRandStr;
      this.testNumhash();
      this.testRandNum();
    }
  }
}

const fileName =
  "20231214_121835_1702556315_821152_00000000000000000002b172e50a5e3e88d4f27a43e692b9631978cc861c28b8.json.gz";
const fileContents =
  '{"chain":"main","blocks":821152,"headers":821152,"bestblockhash":"00000000000000000002b172e50a5e3e88d4f27a43e692b9631978cc861c28b8","difficulty":67305906902031.39,"time":1702556315,"mediantime":1702550146,"verificationprogress":0.9999999097060711,"initialblockdownload":false,"chainwork":"000000000000000000000000000000000000000061034db581df2850e4ee7463","size_on_disk":933076393,"pruned":true,"pruneheight":820644,"automatic_pruning":true,"prune_target_size":1073741824,"warnings":""}';

const fileData: FileData = DrawDataParse.splitFileData(fileName, fileContents);
console.log("解析测试:", fileData);
// const fileNameData: FileNameData = DrawDataParse.splitFileName(fileName);
const draw: Draw = new Draw(fileData.info);
const test = new DrawTest(draw);
test.testAll();
