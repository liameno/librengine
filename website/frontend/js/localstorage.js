//https://www.sohamkamani.com/blog/javascript-localstorage-with-ttl-expiry/
function set_with_expiry(key, value, ttl) {
    const now = new Date()

    const item = {
        value: value,
        expiry: now.getTime() + ttl,
    }

    localStorage.setItem(key, JSON.stringify(item))
}

function get_with_expiry(key) {
    const item_str = localStorage.getItem(key);

    if (!item_str) {
        return null
    }

    const item = JSON.parse(item_str)
    const now = new Date()

    if (now.getTime() > item.expiry) {
        localStorage.removeItem(key)
        return null
    }

    return item.value
}