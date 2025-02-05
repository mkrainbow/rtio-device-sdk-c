
const RTIO_SERVICE = "http://demo.mkrainbow.local:17917";   // Replace with your RTIO service address.
const DEVICE_ID = "cfa09baa-4913-4ad7-a936-3e26f9671b10";   // Replace with your device ID.
const REMOTE_URL = RTIO_SERVICE + "/" + DEVICE_ID;

function logShow(log) {
    var t = document.getElementById("log_area");
    t.value = t.value + log + '\n';
    t.scrollTop = t.scrollHeight;
}

var bt_on = document.getElementById("bt_on");
bt_on.style.color = 'green';
bt_on.onclick = function () {
    if (bt_on.style.color == 'gray') {
        console.log("click invalid");
        logShow("click invalid");
        return;
    }
    console.log("click ON...");
    bt_on.style.color = 'gray';

    logShow("\nPOST To Device");
    logShow("URL: " + REMOTE_URL);
    logShow('BODY: {"method":"copost", "uri":"/switch","id":12667,"data":"b24="}');

    fetch(REMOTE_URL, {
        method: 'POST',
        body: '{"method":"copost", "uri":"/switch","id":12667,"data":"b24="}'
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
            logShow('RESP: ' + JSON.stringify(data));
            bt_on.style.color = 'green';
        });
}

var bt_off = document.getElementById("bt_off");
bt_off.style.color = 'green';
bt_off.onclick = function () {
    if (bt_off.style.color == 'gray') {
        console.log("click invalid");
        logShow("click invalid");
        return;
    }
    console.log("click ON...");
    bt_off.style.color = 'gray';

    logShow("\nPOST To Device");
    logShow("URL: " + REMOTE_URL);
    logShow('BODY: {"method":"copost", "uri":"/switch","id":12667,"data":"b24="}');

    fetch(REMOTE_URL, {
        method: 'POST',
        body: '{"method":"copost", "uri":"/switch","id":12667,"data":"b2Zm"}'
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
            logShow('RESP: ' + JSON.stringify(data));
            bt_off.style.color = 'green';
        });
}
