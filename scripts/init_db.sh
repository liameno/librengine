export URL=http://localhost:8108
export API_KEY=xyz

curl -XDELETE "$URL/collections/websites" -H "X-TYPESENSE-API-KEY: $API_KEY"
curl -XDELETE "$URL/collections/robots" -H "X-TYPESENSE-API-KEY: $API_KEY"

curl -XPOST "$URL/collections/" -d'
{
    "name": "websites",
    "fields": [
      {"name": "title", "type": "string" },
      {"name": "desc", "type": "string" },
      {"name": "url", "type": "string" },
      {"name": "host", "type": "string" },
      {"name": "rating", "type": "int32" },
      {"name": "has_ads", "type": "bool" },
      {"name": "has_analytics", "type": "bool" },
      {"name": "date", "type": "int64" }
    ],
    "default_sorting_field": "date"
}
' -H "X-TYPESENSE-API-KEY: $API_KEY" -H 'Content-Type: application/json'
curl -XPOST "$URL/collections/" -d'
{
    "name": "robots",
    "fields": [
      {"name": "body", "type": "string" },
      {"name": "host", "type": "string" },
      {"name": "date", "type": "int64" }
    ],
    "default_sorting_field": "date"
}
' -H "X-TYPESENSE-API-KEY: $API_KEY" -H 'Content-Type: application/json'
