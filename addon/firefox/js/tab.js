meta_tags = window.document.getElementsByTagName("meta");
content = "";

for (let i = 0; i < meta_tags.length; i++) {
    if (meta_tags[i].name === "description") {
        content = meta_tags[i].content;
    }
}

content;