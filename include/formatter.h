#pragma once

#include <string>
#include <variant>

#include "log_entry.h"

namespace LogAnalyzer {

// JSON formatter: {"timestamp":"...","level":"...","message":"...","module":"..."}
struct JsonFormatter {
    [[nodiscard]] std::string format(const LogEntry& entry) const {
        std::string result = "{\"timestamp\":\"" + entry.formatTimestamp() +
                             "\",\"level\":\"" + std::string(toString(entry.level)) +
                             "\",\"message\":\"" + entry.message + "\"";
        if (entry.module) {
            result += ",\"module\":\"" + *entry.module + "\"";
        } else {
            result += ",\"module\":null";
        }
        result += "}";
        return result;
    }
};

// XML formatter: <entry><timestamp>...</timestamp>...</entry>
struct XmlFormatter {
    [[nodiscard]] std::string format(const LogEntry& entry) const {
        std::string result = "<entry>"
                             "<timestamp>" + entry.formatTimestamp() + "</timestamp>"
                             "<level>" + std::string(toString(entry.level)) + "</level>"
                             "<message>" + entry.message + "</message>";
        if (entry.module) {
            result += "<module>" + *entry.module + "</module>";
        }
        result += "</entry>";
        return result;
    }
};

// PlainText formatter: [YYYY-MM-DD HH:MM:SS] LEVEL [Module] Message
struct PlainTextFormatter {
    [[nodiscard]] std::string format(const LogEntry& entry) const {
        std::string result = "[" + entry.formatTimestamp() + "] " +
                             std::string(toString(entry.level));
        if (entry.module) {
            result += " [" + *entry.module + "]";
        }
        result += " " + entry.message;
        return result;
    }
};

using Formatter = std::variant<JsonFormatter, XmlFormatter, PlainTextFormatter>;

class LogFormatter {
public:
    explicit LogFormatter(Formatter fmt) : fmt_(std::move(fmt)) {}

    [[nodiscard]] std::string format(const LogEntry& entry) const {
        return std::visit([&entry](const auto& f) { return f.format(entry); }, fmt_);
    }

private:
    Formatter fmt_;
};

} // namespace LogAnalyzer
