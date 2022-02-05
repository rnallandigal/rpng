#include <coroutine>

#include <fmt/format.h>

struct task {
	struct promise_type;
	std::coroutine_handle<promise_type> handle;

	operator std::coroutine_handle<promise_type>() const { return handle; }
	operator std::coroutine_handle<>() const { return handle; }

	struct promise_type {
		int val;
		task get_return_object() {
			return { std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void unhandled_exception() {}
		std::suspend_always yield_value(int i) { val = i; return {}; }
	};
};

task nats() {
	for (int i = 0; ; i++) {
		co_yield i;
	}
}

int main(int argc, char ** argv) {
	std::coroutine_handle<task::promise_type> handle = nats();
	for (int i = 0; i < 5; ++i) {
		fmt::print("Caller: {}\n", i);
		handle.resume();
		fmt::print("Received {}\n", handle.promise().val);
	}
	handle.destroy();
	return 0;
}
