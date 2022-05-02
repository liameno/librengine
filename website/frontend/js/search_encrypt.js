let rsa = new JSEncrypt({default_key_size: 1024});
let is_started = true;

let public_key = get_with_expiry("public_key");
let private_key = get_with_expiry("private_key");

if (public_key == null || private_key == null) {
    rsa.getKey(function() {
        let public_key = get_with_expiry("public_key");
        let private_key = get_with_expiry("private_key");

        if (public_key == null) {
            set_with_expiry("public_key", rsa.getPublicKey(), 3600 * 1000); //1 hour
        }
        if (private_key == null) {
            set_with_expiry("private_key", rsa.getPrivateKey(), 3600 * 1000); //1 hour
        }

        set_with_expiry("public_key", rsa.getPublicKey(), 3600 * 1000); //1 hour
        set_with_expiry("private_key", rsa.getPrivateKey(), 3600 * 1000); //1 hour

        is_started = false;
    });
} else {
    is_started = false;
}

function submit_form() {
    if (is_started) return false;
    is_started = true;

    let form = document.getElementById("search_widget");
    let query = document.getElementById("q");
    let encryption = document.getElementById("e");
    let key = document.getElementById("ek");
    let query_v = query.value;
    let key_v = key.value;

    let rsa2 = new JSEncrypt({default_key_size: 1024});
    rsa2.setPublicKey(atob(key_v));

    let encrypted = rsa2.encrypt(query_v);
    let public_key = get_with_expiry("public_key");

    query.value = encrypted;
    encryption.value = "1";
    key.value = btoa(public_key);

    form.submit();
    return false;
}