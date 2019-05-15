#pragma once

#define EXPAND(x) x

#define EVERY_PAIR_ARG_0()
#define EVERY_PAIR_ARG_1(a0_, a0) a0_ a0
#define EVERY_PAIR_ARG_2(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_1(__VA_ARGS__))
#define EVERY_PAIR_ARG_3(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_2(__VA_ARGS__))
#define EVERY_PAIR_ARG_4(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_3(__VA_ARGS__))
#define EVERY_PAIR_ARG_5(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_4(__VA_ARGS__))
#define EVERY_PAIR_ARG_6(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_5(__VA_ARGS__))
#define EVERY_PAIR_ARG_7(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_6(__VA_ARGS__))
#define EVERY_PAIR_ARG_8(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_7(__VA_ARGS__))
#define EVERY_PAIR_ARG_9(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_8(__VA_ARGS__))
#define EVERY_PAIR_ARG_10(a0_, a0, ...) a0_ a0, EXPAND(EVERY_PAIR_ARG_9(__VA_ARGS__))

#define EVERY_SECOND_ARG_0()
#define EVERY_SECOND_ARG_1(a0_, a0) a0
#define EVERY_SECOND_ARG_2(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_1(__VA_ARGS__))
#define EVERY_SECOND_ARG_3(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_2(__VA_ARGS__))
#define EVERY_SECOND_ARG_4(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_3(__VA_ARGS__))
#define EVERY_SECOND_ARG_5(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_4(__VA_ARGS__))
#define EVERY_SECOND_ARG_6(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_5(__VA_ARGS__))
#define EVERY_SECOND_ARG_7(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_6(__VA_ARGS__))
#define EVERY_SECOND_ARG_8(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_7(__VA_ARGS__))
#define EVERY_SECOND_ARG_9(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_8(__VA_ARGS__))
#define EVERY_SECOND_ARG_10(a0_, a0, ...) a0, EXPAND(EVERY_SECOND_ARG_9(__VA_ARGS__))

#define FORWARD_FN_BASE(name, argCount, isConst, ...) \
	auto name ## (EXPAND(EVERY_PAIR_ARG_ ## argCount(__VA_ARGS__))) isConst -> decltype(FORWARD_FN_IMPL(name, EVERY_SECOND_ARG_ ## argCount (__VA_ARGS__))) override final \
	{ \
		return FORWARD_FN_IMPL(name, EVERY_SECOND_ARG_ ## argCount (__VA_ARGS__)); \
	}

#define FORWARD_FN(name, argCount, ...) FORWARD_FN_BASE(name, argCount, , __VA_ARGS__)
#define FORWARD_FN_CONST(name, argCount, ...) FORWARD_FN_BASE(name, argCount, const, __VA_ARGS__)
