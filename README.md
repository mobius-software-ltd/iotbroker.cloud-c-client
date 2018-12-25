# iotbroker.cloud-c-client

### Project description

IoTBroker.cloud C Client is an application that allows you to connect to the server using MQTT, MQTT-SN, 
AMQP or COAP protocols. IoTBroker.cloud C Client gives the opportunity to exchange messages using protocols 
mentioned above. Your data can be also encrypted with **TLS** or **DTLS** secure protocols.   

Below you can find a brief description of each protocol that can help you make your choice. 
If you need to get more information, you can find it in our [blog](https://www.iotbroker.cloud/clientApps/C/MQTT).
 
**MQTT** is a lightweight publish-subscribe based messaging protocol built for use over TCP/IP.  
MQTT was designed to provide devices with limited resources an easy way to communicate effectively. 
You need to familiarize yourself with the following MQTT features such as frequent communication drops, low bandwidth, 
low storage and processing capabilities of devices. 

Frankly, **MQTT-SN** is very similar to MQTT, but it was created for avoiding the potential problems that may occur at WSNs. 

Creating large and complex systems is always associated with solving data exchange problems between their various nodes. 
Additional difficulties are brought by such factors as the requirements for fault tolerance, 
the geographical diversity of subsystems, the presence a lot of nodes interacting with each others. 
The **AMQP** protocol was developed to solve all these problems, which has three basic concepts: 
exchange, queue and routing key. 

If you need to find a simple solution, it is recommended to choose the **COAP** protocol. 
The CoAP is a specialized web transfer protocol for use with constrained nodes and constrained (e.g., low-power, lossy) networks. It was developed to be used in very simple electronics devices that allows them to communicate interactively over the Internet. It is particularly targeted for small low power sensors, switches, valves and similar components that need to be controlled or supervised remotely, through standard Internet networks.   
 
### Installing

* First, you should install [gtk+](https://www.gtk.org/download/index.php) and [libgda](https://packages.ubuntu.com/xenial/libgda-5.0-4) libraries. The following commands should be used for installation:  
```
sudo apt-get install libgtk-3-dev 
```
```
sudo apt-get install libgda-5.0
```

* Then install [uuid-dev](https://www.howtoinstall.co/en/ubuntu/xenial/uuid-dev) and [OpenSSL](https://www.openssl.org/) binary toolkit. The following command should be used for installation:  


```
sudo apt-get install uuid-dev
```
```
sudo apt-get install openssl
```

* Next you should install [Jansson](https://jansson.readthedocs.io/en/latest/gettingstarted.html) and [Libwebsockets](https://libwebsockets.org/lws-api-doc-master/html/md_READMEs_README_8build.html)
libraries following the instructions in links.   
* Then you should install [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/). 
If the above instructions have been followed, you can proceed with the installation of the IoTBroker.Cloud C client.
* To install IoTBroker.Cloud you should clone  iotbroker.cloud-c-client. 
* In order to open iotbroker.cloud-c-client in Eclipse IDE for C/C++ 
you should choose **File** >**Import** > choose **C/C++ folder** > **Existing Code as MakeFile Project** >Next>**Linux GCC**.  
* Then you have to press Browse button to indicate the path to the root directory and then click the Finish button. 
* Next you should go Help> Eclipse Market, where you should find Pkg-config support for Eclipse CDT and install it.  
* Then you should highlight iotbroker.cloud-c-client in the Project tab in the upper right corner 
and choose **File** > **Properties >C/C++ build**> **Settings, and set the following flags**: 
gtk+-3.0, libgda-5.0, libgda-sqlite-5.0, jansson, openSSL and libwebsockets, then click **Apply** and **Close** button. 
* Next set the **Generate Makefiles automatically flag**. (File >C/C++ Build > Makefile generation > set the flag);
* Then add **-pthread flag**. (File > C/C++ Build >Settings >Miscellaneous > Linker flags > -pthread);

When you finished with installation, you can launch IoTBroker.Cloud. 
In order to start the application, you should set the cursor on the iotbroker.cloud-c-client, 
right-click **Run as** > **Build Project** >**2Local C/C++ Application**.

If the procedure is successful, you will see the Login page in the form of pop-up window.  
Please note that at this stage it is not possible to register as a client. You can only log in to your existing account. 

IoTBroker.Cloud C Client is developed by [Mobius Software](https://www.mobius-software.com/).

## [License](LICENSE.md)
