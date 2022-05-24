function decrypt() {
    let center_container = document.getElementsByClassName("results")[0];
    let splited = center_container.innerHTML.split('\n');
    let result = "";
    splited.shift();

    let rsa = new JSEncrypt();
    rsa.setPrivateKey(get_with_expiry("private_key"));

    for (let i = 0; i < splited.length; i++) {
        let item = splited[i];

        item = item.replaceAll("\n", "");
        item = item.trim();

        let decrypted = rsa.decrypt(item);
        if (decrypted == null) continue;

        result += decrypted;
    }

    center_container.innerHTML = result;
}