/*!
 *  @file       btterminal.cpp
 *  Project     Blackstomp Arduino Library
 *  @brief      Blackstomp Library for the Arduino
 *  @author     Hasan Murod
 *  @date       19/11/2020
 *  @license    MIT - Copyright (c) 2020 Hasan Murod
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "btterminal.h"

//#define CUSTOMDEBUG

//static variables
static bt_terminal* btterminal=NULL;
static TaskHandle_t bleServiceHandle=NULL;
static void bleservice_task(void* arg);
static uint32_t passkey = 123456; // BLE PASS

std::string to_string(const char* format, int x ) {
  int length = snprintf( NULL, 0,format, x );
  assert( length >= 0 );
  char* buf = new char[length + 1];
  snprintf( buf, length + 1, format, x );
  std::string str( buf );
  delete[] buf; 
  return str;
}

//server callback: onConnect, onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      pServer->updatePeerMTU(pServer->getConnId(),btterminal->mtu);
      btterminal->deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      btterminal->deviceConnected = false;
    }
};

class MySecurity : public BLESecurityCallbacks {

	uint32_t onPassKeyRequest(){
		
		#ifdef CUSTOMDEBUG
        printf("PassKeyRequest\n");
        #endif
        
		return btterminal->module->bleTerminal.passKey;
	}
	void onPassKeyNotify(uint32_t pass_key){
		
		#ifdef CUSTOMDEBUG
        printf("The passkey Notify number:%d\n", pass_key);
        #endif
	}
	bool onConfirmPIN(uint32_t pass_key){
        
        #ifdef CUSTOMDEBUG
        printf("The passkey YES/NO number:%d\n", pass_key);
        #endif
        btterminal->authenticated = false;
	    vTaskDelay(5000);
		return true;
	}
	bool onSecurityRequest(){
		
		#ifdef CUSTOMDEBUG
	    printf("SecurityRequest\n");
	    #endif
	    
		return true;
	}

	void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
		
		if(cmpl.success)
		{
			btterminal->authenticated = true;
			#ifdef CUSTOMDEBUG
			printf("Starting BLE work!\n");
			#endif
		}
		else
		{
			btterminal->authenticated = false;
			#ifdef CUSTOMDEBUG
			printf("BLE authentication fails!\n");
			#endif
		}
	}
};

//######################################################################
static void bleservice_task(void* arg)
{
	char response[512];
	while(true)
	{
		int notifycount = ulTaskNotifyTake( pdTRUE, 10000 );
		if(notifycount==0)
		{
			continue; //no notification timeout
		}
		
		//process the btterminal->request
		response[0]=0; //initialize with null string
		btterminal->module->onBleTerminalRequest(btterminal->request.c_str(),response);
		if(response[0]!=0)
		{
			btterminal->sendresponse(response);
		}
	}
	vTaskDelete(NULL);
}

//######################################################################
//BLE CHARACTERISTIC WRITE CALLBACK
class writecallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) 
    {
		if(btterminal->authenticated)
		{
			btterminal->request = pCharacteristic->getValue();
			#ifdef CUSTOMDEBUG
			printf("%s\n",btterminal->request.c_str());
			#endif
			xTaskNotifyGive(bleServiceHandle);
		}
		#ifdef CUSTOMDEBUG
		else
		{
			btterminal->request = pCharacteristic->getValue();
			printf("Unauthenticated request: %s\n",btterminal->request.c_str());
		}
		#endif
      
      
    }
};

int wtick = 0;
void btconnectionwatch_task(void* arg)
{
  bt_terminal* t = (bt_terminal*)arg;
	while(true)
	{
		// disconnecting
		if (!t->deviceConnected && t->oldDeviceConnected) {
			vTaskDelay(500); // give the bluetooth stack the chance to get things ready
			t->pServer->startAdvertising(); // restart advertising
			t->oldDeviceConnected = t->deviceConnected;
		}
		// connecting
		if (t->deviceConnected && !t->oldDeviceConnected) {
			// do stuff here on connecting
			t->oldDeviceConnected = t->deviceConnected;
		}
		vTaskDelay(100);
	}
}

bt_terminal::bt_terminal()
{
  processrequest = NULL;
  btterminal = this;
  mtu = 517;
  pServer = NULL;
  pCharacteristic = NULL;
  deviceConnected = false;
  oldDeviceConnected = false;
  authenticated = false;
  request.reserve(518);
}

void bt_terminal::begin(const char* device_name, const char* service_uuid, const char* characteristic_uuid, uint32_t pass_key, int priority)
{	
	xTaskCreatePinnedToCore(bleservice_task, "bleservice_task",4096,(void*)this,priority,&bleServiceHandle,0);
	
	BLEDevice::init(device_name);
	BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
	BLEDevice::setSecurityCallbacks(new MySecurity());
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());
    
	BLEService *pService = pServer->createService(service_uuid);
	pCharacteristic = pService->createCharacteristic(
										 characteristic_uuid,
										 BLECharacteristic::PROPERTY_WRITE |
										 BLECharacteristic::PROPERTY_READ |
										 BLECharacteristic::PROPERTY_NOTIFY |
										 BLECharacteristic::PROPERTY_INDICATE
									   );
	pCharacteristic->setCallbacks(new writecallback());

	// https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
	// Create a BLE Descriptor
	pCharacteristic->addDescriptor(new BLE2902());
	pService->start();
	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(service_uuid);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
	pAdvertising->setMinPreferred(0x12);
	BLEDevice::startAdvertising();
	
	BLESecurity *pSecurity = new BLESecurity();

    passkey = pass_key;
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
	
	pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
	
	pSecurity->setCapability(ESP_IO_CAP_OUT);
	
	pSecurity->setKeySize(16);
	
	uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
	esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
	
	pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
	
	uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
	esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
	
	xTaskCreatePinnedToCore(btconnectionwatch_task, "btconnectionwatch_task",4096,(void*)this,priority,NULL,0);
}

void bt_terminal::sendresponse(std::string responseStr)
{
  responseStr += "\n";
  pCharacteristic->setValue(responseStr.c_str());
  pCharacteristic->notify();
}
