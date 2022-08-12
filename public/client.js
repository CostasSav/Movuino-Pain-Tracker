console.log('Client-side code running');

var deviceName = 'Palm'
var bleService = 'environmental_sensing'
var bleCharacteristic = 'uv_index'
var bleCharacteristic2 = ''
var bluetoothDeviceDetected
var gattCharacteristic

var flag0 = 1;
var flag1 = 0;
var flag2 = 0;

var time1 = "00000000";
var time2 = "00000000";

let intensity = 0;

//look for devices
document.querySelector('#read').addEventListener('click', function () {
    if (isWebBluetoothEnabled()) { read() }
});

//start reading device data
document.querySelector('#start').addEventListener('click', function (event) {
    if (isWebBluetoothEnabled()) { start() }
});

//stop reading device data
document.querySelector('#stop').addEventListener('click', function (event) {
    if (isWebBluetoothEnabled()) { stop() }
});

//check if browser is bluetooth compatible
function isWebBluetoothEnabled() {
    if (!navigator.bluetooth) {
        console.log('Web Bluetooth API is not available in this browser!')
        return false
    }

    return true
};

//gets info of surrounding devices
function getDeviceInfo() {
    let options = {
        optionalServices: [bleService],
        filters: [
        { "name": deviceName }
        ]
    }

    console.log('Requesting any Bluetooth Device...')
    return navigator.bluetooth.requestDevice(options).then(device => {
        bluetoothDeviceDetected = device
    }).catch(error => {
        console.log('Argh! ' + error)
    })
};


function read() {
    return (bluetoothDeviceDetected ? Promise.resolve() : getDeviceInfo())
    .then(connectGATT)
    .then(_ => {
        console.log('Reading Pain Index...')
        return gattCharacteristic.readValue()
    })
    .catch(error => {
        console.log('Waiting to start reading: ' + error)
    })
};

function connectGATT() {
    if (bluetoothDeviceDetected.gatt.connected && gattCharacteristic) {
        return Promise.resolve()
    }

    return bluetoothDeviceDetected.gatt.connect()
    .then(server => {
        console.log('Getting GATT Service...')
        return server.getPrimaryService(bleService)
    })
    .then(service => {
        console.log('Getting GATT Characteristic...')
        return service.getCharacteristic(bleCharacteristic)
    })
    .then(characteristic => {
        gattCharacteristic = characteristic
        gattCharacteristic.addEventListener('characteristicvaluechanged',
            handleChangedValue)
        document.querySelector('#start').disabled = false
        document.querySelector('#stop').disabled = true
    })
};

function handleChangedValue(event) {
    //reception of data via bluetooth is done in three stages
    let value = event.target.value.getUint8(0);

    if (flag0 == 1) {  //1st stage: reception of the pain value
        flag0 = 0;
        flag1 = 1;
        flag2 = 0;

        pain = value;
    } else if (flag1 == 1) { //2nd stage: reception of first part of the date
        flag0 = 0;
        flag1 = 0;
        flag2 = 1;

        time1 = value.toString(2);
    }else if (flag2 = 1) { //3rd stage: reception of second part of the date
        flag0 = 1;
        flag1 = 0;
        flag2 = 0;

        time2 = value.toString(2);
        var time3 = parseInt(time1 + time2, 2);
        //console.log(time3);
        //console.log(1659000000 + (time3 * 1000));


        painDate = new Date(((1659000000 + time3)*1000));

        var painValue = {
            intensity: pain,
            date: painDate
        }

        fetch('/', { 
            method: 'POST',
            headers: {
                "Accept": "application/json, text/plain, */*",
                "Content-type": "application/json; charset=UTF-8"
            },
            body: JSON.stringify(painValue)
        })
        .then(response => {
            if (!response.ok) {
                throw new Error("HTTP error " + response.status);
            }
            let dataReceived = response.json();
        })
        .then(dataReceived => {
            console.log(dataReceived);
        });
    }
};

function start() {
    gattCharacteristic.startNotifications()
    .then(_ => {
        console.log('Start reading...')
        document.querySelector('#start').disabled = true
        document.querySelector('#stop').disabled = false
    })
    .catch(error => {
        console.log('[ERROR] Start: ' + error)
    })
};

function stop() {
    gattCharacteristic.stopNotifications()
    .then(_ => {
        console.log('Stop reading...')
        document.querySelector('#start').disabled = false
        document.querySelector('#stop').disabled = true
    })
    .catch(error => {
        console.log('[ERROR] Stop: ' + error)
    })
};

