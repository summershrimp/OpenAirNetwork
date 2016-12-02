#AirNetwork

## 编译：
需要安装libyaml库。Ubuntu/Debian下使用`apt-get install libyaml-dev`, CentOS/RHEL下使用`yum install libyaml-devel`安装。
```
make
```
或打开停止等待协议
```
make EXTRA_CFLAGS=-DENABLE_ARQ
```
随后会在bin/目录下生成可执行文件an。

## 运行
将config.yml.example复制为config.yml，并保证在运行目录中。
```
port: 5554   #UDP 监听端口
master: true #是否为主节点， 主节点将广播所有收到的数据包。
crafts:      #其他节点IP地址
    - 192.168.59.3   
    - 192.168.59.4
    - 192.168.59.5

protos: 	#UAVMP协议字段type值与对应的fifo管道路径， 其余服务程序可以通过读写相应的fifo管道进行数据收发。
    - id: 1
      fifo_in: /tmp/fifo1in         #fifo_in 为服务程序的读管道
      fifo_out: /tmp/fifo1out       #fifo_out 为服务程序的写管道
    - id: 2
      fifo_in: /tmp/fifo2in
      fifo_out: /tmp/fifo2out
```

