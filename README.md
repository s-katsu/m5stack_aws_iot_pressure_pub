# m5stack_aws_iot_pressure_pub
気圧センサーと通信するM5StackのAWS IOTのpublisher

## 概要
センサー情報などをAWS IOT経由でアップロードするためのMQTT Client

## 構成
HW構成はM5Stack + 2SMPB-02E絶対圧センサーを想定する
ソースはArduinoを想定

![image](https://user-images.githubusercontent.com/20053970/170853366-0058c24d-4f6b-4d95-bb21-c6a038afb3e6.png)

## センサーライブラリ
絶対圧センサー 2smpb02e-grove-m5stackのライブラリ

[2smpb02e-grove-m5stack)](https://github.com/omron-devhub/2smpb02e-grove-m5stack)



## 機能
- 10分間隔でAWS IoTに圧力情報[hPa]をpublishする

- Lcd上には1分おきに更新した気圧を表示する

## 認証情報
src/cert.hに証明書などのAWS IoTにアクセスするための情報を記載する

```
const char *root_ca = R"(-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)";

const char *certificate = R"(-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)";

const char *private_key = R"(-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)";
```

その他AWS IoTにアクセスするための情報やWiFiのAPの設定はsrc/config.hに記載する

```
//WiFi
const char* SSID = "xxxxxxxxxx";
const char* PASS = "xxxxxxxx";

//AWS IOT
const char* AWS_IOT_ENDPOINT = "xxxxxxxxx";
const uint32_t AWS_IOT_PORT = 8883;
const char* THING_NAME = "xxx_sensor";
const char* TOPIC = "sensor/device_name/sensor_update";
```
