# serialcom
QT串口通信程序
通信协议
2016年8月

一、协议介绍
本协议规定了程序与手持机之间的通信规程。
硬件接口为RS232串行接口，波特率为115200bps，8位数据位，1位停止位，无校验位，无流控。
数据传送为双向。
二、命令控制字列表
子命令	功能描述
01	用户登录
02	调度信息查询
03	设备出库
04	设备入库
05	设备安设
06	设备撤除
07	图片上传
08	股道查询
51	消息返回
52	错误消息返回
64	推送调度信息


三、协议内容
1.用户登录
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	01	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	“用户名~密码”		自定义长度（与data_size保持一致），使用“~”分隔
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

后台系统回复：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	01	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	 Json格式数据{"info":"无此用户","suc":false,"uid":"qwerrtuajjaslkko","uname":"silen"}		自定义长度（与data_size保持一致），使用“~”分隔
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：登录信息包含用户名和密码，命令字采用两个字节。


2.调度信息查询
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	02	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	用户ID		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

后台系统回复：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	02	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	Json格式数据
{"dis"："{dispatch_id(调度ID,string),track_id(股道ID,string),dis_state(调度状态,int)}，
{dispatch_id(调度ID,string),track_id(股道ID,string),dis_state(调度状态,int)}"
}		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：调度信息包含用户ID，命令字采用两个字节。

3.设备出库
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	03	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	库房ID~设备ID		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

后台系统回复：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	03	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	Json格式数据
{"suc":"true","device_id":"qweqerwqeer"}		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：出库包含设备ID，命令字采用两个字节。
4.设备入库
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	04	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	库房ID~设备ID		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

后台系统回复：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	04	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	Json格式数据
{"suc":"true","device_id":"qweqerwqeer"}		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：入库包含设备ID，命令字采用两个字节。
5.设备安设
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	05	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	调度ID~用户ID~设备ID~GPS数据		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

后台系统回复：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	05	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	Json格式数据
{"suc":"true","dis_id":"qweqerwqeer"}		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：设备安设GPS数据采用逗号连接作为一个字段进行传输。

6.设备撤除
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	06	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	调度ID~用户ID~设备ID~GPS数据		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

后台系统回复：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	06	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	Json格式数据
{"suc":"true","dis_id":"qweqerwqeer"}		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：设备撤除GPS数据采用逗号连接作为一个字段进行传输。


7.上传图片
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	07	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	调度ID+图片数据(第一包为调度ID+0或调度ID+1，第二包起发送图片数据)		自定义长度（与data_size保持一致）
数据校验位（crc）	04	4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：图片数据头尾各去掉两个字节，命令字采用两个字节。

8.股道查询
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	08	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	用户ID		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

后台系统回复：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	08	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	Json格式数据
{股道ID，股道名}		自定义长度（与data_size保持一致）
数据校验位（crc）		4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	


9.返回消息
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	51	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)		4HEX	收到的CRC校验值
数据校验位（crc）	04	4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：命令字采用两个字节。

10.返回错误消息
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	52	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)		4HEX	计算的CRC校验值
数据校验位（crc）	04	4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：命令字采用两个字节。
11.推送消息
手持机查询：
字段	定义	长度（字节）	描述
包头(pck_header)	0xFFD8	2HEX	
有效数据长度(data_size)		4HEX	
命令字	64	2HEX	
当前包编号(cur_no)		3HEX	
总包数(total_num)		3HEX	
数据体(data)	Json格式数据
{dispatch_id(调度ID,string),track_id(股道ID,string),dis_state(调度状态,int),tn(股道名,string)}	4HEX	计算的CRC校验值
数据校验位（crc）	04	4HEX	采用累加计算的CRC校验值
包尾（pck_tail）	0xFFD9	2HEX	

解释：命令字采用两个字节。
