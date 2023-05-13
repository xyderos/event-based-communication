#include <check.h>

START_TEST(TEST_MULTIPLICATION_SHOULD_BE_OKAY)
{
	int result = multiplication(1, 2);

	ck_assert_int_eq(2, result);
}
END_TEST

START_TEST(TEST_ADDITION_SHOULD_BE_OKAY)
{
	int result = addition(1, 1);

	ck_assert_int_eq(2, result);
}
END_TEST
