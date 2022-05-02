function decrypt() {
    let center_container = document.getElementsByClassName("center_container")[0];
    let splited = center_container.innerHTML.split('\n');
    let result = "";
    splited.shift();

    let last_i;

    let rsa = new JSEncrypt({default_key_size: 1024});
    rsa.setPrivateKey(get_with_expiry("private_key"));

    for (let i = 0; i < splited.length; i++) {
        let item = splited[i];

        if (splited[i] === "") {
            last_i = i;
            break;
        }

        item = item.replaceAll("\n", "");
        item = item.trim();

        let decrypted = rsa.decrypt(item);
        if (decrypted == null) continue;

        result += decrypted;
    }

    splited = splited.slice(last_i);
    let splited_s = splited.join('\n');

    center_container.innerHTML = result + splited_s;
}