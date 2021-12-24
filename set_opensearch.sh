export ES_URL=localhost:9200
curl -XDELETE "$ES_URL/website"
curl -XPUT "$ES_URL/website" -d'{
  "settings": {
    "analysis": {
      "analyzer": {
        "autocomplete": {
          "tokenizer": "autocomplete",
          "filter": [
            "lowercase"
          ]
        },
        "autocomplete_search": {
          "tokenizer": "lowercase"
        },
        "not_analyzed": {

        }
      },
      "tokenizer": {
        "autocomplete": {
          "type": "edge_ngram",
          "token_chars": [
            "letter"
          ]
        }
      }
    }
  },
  "mappings": {
    "properties": {
      "title": {
        "type": "text",
        "analyzer": "autocomplete"
      },
      "content": {
        "type": "text",
        "analyzer": "autocomplete"
      },
      "desc": {
        "type": "text",
	"analyzer": "autocomplete"
      },
      "url": {
        "type": "keyword"
      },
      "host": {
        "type": "keyword"
      },
      "rating": {
        "type": "byte"
      },
      "date": {
        "type": "date"
      }
    }
  }
}
' -H 'Content-Type: application/json'

curl -XDELETE "$ES_URL/robots_txt"
curl -XPUT "$ES_URL/robots_txt" -d'{
  "mappings": {
    "properties": {
      "body": {
        "type": "keyword"
      },
      "host": {
        "type": "keyword"
      },
      "date": {
        "type": "date"
      }
    }
  }
}
' -H 'Content-Type: application/json'
