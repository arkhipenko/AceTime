test_data_generator.out: test_data_generator.cpp
	$(CXX) -Wall -Wextra -std=gnu++11 -o $@ $^

json: test_data_generator.out small.txt
	./test_data_generator.out \
		--tz_version 2019c \
		--start_year 2000 \
		--until_year 2038 \
		< small.txt

clean:
	rm -f test_data_generator.out validation_data.json

