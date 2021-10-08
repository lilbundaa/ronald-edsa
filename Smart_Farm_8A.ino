#include "UbidotsESPMQTT.h" //Library untuk Ubidots MQTT ESP
#include <NewPing.h> //Library untuk HC-SR04
#include <DHT.h> //Library untuk DHT11
#include <MQUnifiedsensor.h> // Library untuk MQ0135
#include <Servo.h> //Library untuk Servo
 
#define TRIGGER_PIN D5 //Pin Trigger HC-SR04 pada NodeMCU
#define ECHO_PIN D6 //Pin Echo HC-SR04 pada NodeMCU
#define MAX_DISTANCE 250 //Maksimum Pembacaan Jarak (cm)
#define DHTPIN D4 //Define Pin DHT
#define DHTTYPE DHT11 //Define Jenis DHT
#define Pompa D0 //Define Pin Relay 1
#define Minum D1 //Define Pin Relay 2
#define LED D2 //Define Pin LED
 
#define placa "ESP8266" //Define jenis board yang digunakan
#define Voltage_Resolution 5 //Tegangan yang digunakan
#define pin A0 //Pin yang digunakan untuk MQ-135
#define type "MQ-135" //Jenis MQ yang digunakan
#define ADC_Bit_Resolution 10 //Resolusi Bit ADC
#define RatioMQ135CleanAir 3.6 //Nilai Udara dianggap bersih
 
#define TOKEN "BBFF-hDWhKN8NxHQjrGpyB0Ql2byy0XSKBu" //Token dari Ubidots
#define WIFINAME "Danns" //SSID Wi-Fi
#define WIFIPASS "danishaja" //Password Wi-Fi
 
MQUnifiedsensor MQ135(placa, Voltage_Resolution,ADC_Bit_Resolution, pin, type); //Aktifkan fungsi MQ
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //Aktifkan Fungsi HCSR-04
DHT dht(DHTPIN, DHTTYPE); //Aktifkan Fungsi DHT
Servo servo; //Aktifkan fungsi Servo
Ubidots client(TOKEN); //Aktifkan fungsi Ubidots dengan Token
 
unsigned long durasiKirim = 0;
unsigned long jedaKirim = 2000;
 
//Untuk menerima perintah dari Dashboard Ubidots dengan isi Topik dan Nilai Pesannya
void callback(char* topic, byte* payload, unsigned int length) {
//Mencetak Topik dan Nilai Pesan yang masuk dari Dashboard Ubidots
 Serial.print("Pesan Diterima: [");
 Serial.print(topic);
 Serial.print("] ");
 for (int i=0;i<length;i++) {
 Serial.print((char)payload[i]);
 }
 
//Kondisi apabila Topik yang masuk LED dan Membaca Nilai Pesan yang dikirimkan untuk menyalakan LED
if(strcmp(topic,"/v1.6/devices/smart-farm/led/lv")==0){
 if((char)payload[0]=='1'){
 digitalWrite(LED, HIGH);
 }
 else{
 digitalWrite(LED, LOW);
 }
 Serial.println();
}
 
//Kondisi apabila Topik yang masuk Relay 1 dan Membaca Nilai Pesan yang dikirimkan untuk menyalakan Pompa
if(strcmp(topic,"/v1.6/devices/smart-farm/relay1/lv")==0){
 if((char)payload[0]=='0'){
 digitalWrite(Pompa, LOW);
 }
 else{
 digitalWrite(Pompa, HIGH);
 }
 Serial.println();
}
 
//Kondisi apabila Topik yang masuk Relay 2 dan Membaca Nilai Pesan yang dikirimkan untuk Menyalakan Air
if(strcmp(topic,"/v1.6/devices/smart-farm/relay2/lv")==0){
 if((char)payload[0]=='0'){
 digitalWrite(Minum, LOW);
 }
 else{
 digitalWrite(Minum, HIGH);
 }
 Serial.println();
}
 
//Kondisi apabila Topik yang masuk Servo dan Membaca Nilai Pesan yang dikirimkan untuk Pemberian Pakan
if(strcmp(topic,"/v1.6/devices/smart-farm/servo/lv")==0){
 if((char)payload[0]=='1'){
 servo.write(180); //Menggerakkan Servo ke Sudut 180 apabila pesan yang masuk bernilai 1
 }
 else{
 servo.write(0); //Menggerakkan Servo ke Sudut 0 apabila pesan yang masuk bernilai 0
 }
 Serial.println();
}
}
 
 
void setup() {
Serial.begin(115200); //Baudrate untuk Serial Komunikasi
client.ubidotsSetBroker("industrial.api.ubidots.com"); //Set Broker Ubidots
client.setDebug(true);
client.wifiConnection(WIFINAME, WIFIPASS); //Mencoba Koneksi dengan Wi-Fi
client.begin(callback); //Aktifkan fungsi Callback
pinMode(LED, OUTPUT); //Set LED sebagai Output
pinMode(Pompa, OUTPUT); //Set Relay 1 sebagai Output
pinMode(Minum, OUTPUT); //Set Relay 2 sebagai Output
digitalWrite(LED, LOW); //Set LED dengan Nilai LOW
digitalWrite(Pompa, HIGH); //Set Relay 1 dengan Nilai HIGH
digitalWrite(Minum, HIGH); //Set Relay 2 dengan Nilai HIGH
 
client.ubidotsSubscribe("smart-farm","led"); //Subscribe ke Topik LED dan Device Smart-Farm
client.ubidotsSubscribe("smart-farm","relay1"); //Subscribe ke Topik Relay1 dan Device Smart-Farm
client.ubidotsSubscribe("smart-farm","relay2"); //Subscribe ke Topik Relay2 dan Device Smart-Farm
client.ubidotsSubscribe("smart-farm","servo"); //Subscribe ke Topik Servo dan Device Smart-Farm
 
dht.begin(); //Memulai fungsi DHT
servo.attach(13); //Memulai fungsi Servo pada Pin 13
MQ135.setRegressionMethod(1); //_PPM = a*ratio^b //Set Method yang digunakan untuk MQ-135
MQ135.init(); //Memulai fungsi MQ-135
//Fungsi untuk Kalibrasi MQ-135
 float calcR0 = 0;
 for(int i = 1; i<=10; i ++)
 {
 MQ135.update();
 calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
 Serial.print(".");
 }
 MQ135.setR0(calcR0/10);
}
 
void loop() {
unsigned long time = millis(); //Fungsi Millis
MQ135.update(); //Update Nilai MQ-135
MQ135.setA(102.2 ); MQ135.setB(-2.473); //Set Nilai Sampling untuk NH4 /Amonia
float NH4 = MQ135.readSensor(); //Hasil Pembacaan nilai Amonia
int jarak = sonar.ping_cm(); //Hasil Pembacaan Jarak
float h = dht.readHumidity(); //Hasil Pembacaan Humidity
float t = dht.readTemperature(); //Hasil Pembacaan Suhu
 
//Reconnect apabila Sambungan Koneksi NodeMCU terputus dan resubscribe topik dan device yang ada
 if(!client.connected()){
 client.reconnect();
 client.ubidotsSubscribe("smart-farm","led");
 client.ubidotsSubscribe("smart-farm","relay1");
 client.ubidotsSubscribe("smart-farm","relay2");
 client.ubidotsSubscribe("smart-farm","servo");
 }
 
//Kondisi untuk mengirimkan hasil pembacaan sensor ke Device Ubidots setiap 2 detik
 if ((unsigned long)(time - durasiKirim) >= jedaKirim)
 {
 //Mengirimkan hasil pembacaan sensor ke Device Ubidots
 client.add("ketinggian", jarak);
 client.add("suhu", t);
 client.add("kelembapan", h);
 client.add("amonia", NH4);
 client.ubidotsPublish("smart-farm");
 durasiKirim = millis();
 }
 client.loop();
}
