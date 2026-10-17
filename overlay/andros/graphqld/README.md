# graphqld

Vendor init service (`graphqld.rc`) that serves the store over HTTP on port 8000.

- `POST /graphql` for queries and mutations; `GET /graphql?query=subscription{...}` streams server-sent events
- `GET /` serves a tiny in-browser console (`graphiql.h`)
- Files: `http` (HTTP), `gql_lexer/parser/exec` (GraphQL engine), `json*` (JSON), `resolvers` (schema from the
  generated catalog: a field and a `set<Name>` mutation per sensor), `sse` (subscriptions), `handler` (routing)

Host-tested by `tests/cpp/test_graphql.cpp` (`make test-cpp`).
