#pragma once

namespace fmt {

struct base_formatter {
	format_parse_context::iterator parse(format_parse_context & ctx) {
		return std::find(ctx.begin(), ctx.end(), '}');
	}
};

}
