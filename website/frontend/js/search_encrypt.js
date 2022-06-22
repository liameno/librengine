let rsa = new JSEncrypt({default_key_size: 1024});
let is_generating = false;

let public_key = get_with_expiry("public_key");
let private_key = get_with_expiry("private_key");

function load() {
    is_generating = true;

    const title = document.getElementsByClassName("title")[0];
    title.innerHTML = "GENERATING...";

    if (public_key == null || private_key == null) {
    rsa.getKey(function() {
        const expire_time = 3600 * 1000; //1 hour

        set_with_expiry("public_key", rsa.getPublicKey(), expire_time);
        set_with_expiry("private_key", rsa.getPrivateKey(), expire_time);

        title.innerHTML = "librengine";
        is_generating = false;
    });
    } else {
        title.innerHTML = "librengine";
        is_generating = false;
    }
}

function submit_form() {
    if (is_generating) return false;
    is_generating = true;

    let form = document.getElementById("search_widget");
    let query = document.getElementById("q");
    let encryption = document.getElementById("e");
    let key = document.getElementById("ek");
    let query_v = query.value;
    let key_v = key.value;

    let rsa2 = new JSEncrypt();
    rsa2.setPublicKey(atob(key_v));

    let encrypted = rsa2.encrypt(query_v);
    let public_key = get_with_expiry("public_key");

    query.value = encrypted;
    encryption.value = "1";
    key.value = btoa(public_key);

    form.submit();

    return false;
}
