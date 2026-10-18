# Common entry points. Everything heavy runs in Docker; see README.md.
GEN := build/gen
CXXFLAGS := -std=c++17 -Wall -Wextra -I$(GEN) -Ioverlay/andros/store -Ioverlay/andros/graphqld
CORE := overlay/andros/store/store.cpp $(filter-out %/main.cpp,$(wildcard overlay/andros/graphqld/*.cpp))

.PHONY: cloud test-nmea test-poller help build run verify shell quick test test-cpp gen lint clean

help:
	@echo "make build   - sync + patch + build the Android image in Docker (hours, first time)"
	@echo "make run     - boot it; GraphQL on http://localhost:8000"
	@echo "make verify  - end-to-end check against the running device"
	@echo "make quick   - rebuild only our modules"
	@echo "make test    - python + C++ unit tests, lint, 100-line check"

build:
	docker compose --profile build run --rm builder asp all
cloud:
	uv run asp cloud build
run:
	docker compose up android
verify:
	docker compose exec android asp verify
quick:
	docker compose --profile build run --rm builder asp build --quick
shell:
	docker compose --profile build run --rm builder bash

gen:
	uv run asp render --out $(GEN)
test-cpp: gen
	g++ $(CXXFLAGS) tests/cpp/test_graphql.cpp $(CORE) -o build/test_graphql -pthread
	build/test_graphql
HAL := overlay/andros/sensors_subhal
test-poller: gen
	g++ $(CXXFLAGS) -Itests/cpp/stubs -I$(HAL) tests/cpp/test_poller.cpp $(HAL)/poller.cpp $(HAL)/poller_emit.cpp \
		$(HAL)/sensor_table.cpp overlay/andros/store/store.cpp -o build/test_poller -pthread
	ANDROS_STORE=build/poller.store build/test_poller
GNSS := overlay/andros/gnss
test-nmea:
	g++ $(CXXFLAGS) -I$(GNSS) tests/cpp/test_nmea.cpp $(GNSS)/nmea.cpp $(GNSS)/sats.cpp -o build/test_nmea
	build/test_nmea
test: test-cpp test-poller test-nmea lint
	uv run pytest -q
lint:
	uv run ruff check .
	python3 scripts/check_loc.py .
clean:
	rm -rf build
