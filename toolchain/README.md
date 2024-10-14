# 交叉编译链安装说明

## 解压

使用以下命令将压缩文件 `toolchain-sunxi-musl-gcc-830.tar.gz` 解压到 `~/` 目录下：
   ```bash
   tar -xzf toolchain-sunxi-musl-gcc-830.tar.gz -C ~/
   ```
## 配置环境变量

1.打开或编辑 ~/.profile 文件：
   ```bash
   vim ~/.profile
   ```
2.在 .profile 文件末尾添加以下行
   ```bash
   export STAGING_DIR=~/toolchain-sunxi-musl-gcc-830/toolchain/arm-openwrt-linux-muslgnueabi:$STAGING_DIR
   ```
3.生效环境变量
   ```bash
   source ~/.profile
   ```
