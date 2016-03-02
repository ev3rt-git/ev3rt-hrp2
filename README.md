# Usage

Clone repositories:
```sh
git clone https://github.com/ev3rt-git/ev3rt-hrp2-base.git ev3rt-base
git clone https://github.com/ev3rt-git/ev3rt-hrp2-sdk ev3rt-sdk
```

Build cfg:
```sh
cd ev3rt-base/cfg
make
```

Build application loader:
```sh
cd ev3rt-base/base-workspace
make app=loader
```

Build Hello EV3 (dynamic):
```sh
cd ev3rt-sdk/workspace
make app=helloev3
```

Build Hello EV3 (standalone):
```sh
cd ev3rt-sdk/workspace
make img=helloev3
```
# Build status

Status               | Platform
--------------       | ------ 
[![Build Status](https://travis-ci.org/ev3rt-git/ev3rt-hrp2-base.svg?branch=master)](https://travis-ci.org/ev3rt-git/ev3rt-hrp2-base) | Ubuntu 14.04
