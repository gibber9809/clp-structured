#ifndef CLP_STRUCTURED_SEARCH_INTEGRAL_H
#define CLP_STRUCTURED_SEARCH_INTEGRAL_H

#include <memory>
#include <string>
#include <variant>

#include "Literal.hpp"

namespace clp_structured {
typedef std::variant<int64_t, double> Integral64;

// FIXME: figure out why String types are part of this bitmask
constexpr LiteralTypeBitmask cIntegralLiteralTypes = cIntegralTypes | VarStringT;

/**
 * Class for Integral values (float/int) in the search AST
 */
class Integral : public Literal {
public:
    // Deleted copy
    Integral(Integral const&) = delete;
    Integral& operator=(Integral const&) = delete;

    /**
     * Create an Integral literal from an integral value
     * @param v the value
     * @return an Integral literal
     */
    static std::shared_ptr<Literal> create_from_float(double v);
    static std::shared_ptr<Literal> create_from_int(int64_t v);

    /**
     * Try to create an integral literal from a string
     * @param v the string we are attempting to convert to Integral
     * @return an Integral literal, or nullptr if the string does not represent an integral
     */
    static std::shared_ptr<Literal> create_from_string(std::string const& v);

    /**
     * Strict checks for type matching against a given literal type.
     * @return true if the check succeeds
     */
    bool matches_type(LiteralType type) override { return type & cIntegralLiteralTypes; }

    bool matches_any(LiteralTypeBitmask mask) override { return mask & cIntegralLiteralTypes; }

    bool matches_exactly(LiteralTypeBitmask mask) override { return mask == cIntegralLiteralTypes; }

    void print() override;

    /**
     * Return the underlying integral value
     * @return the underlying integral value
     */
    Integral64 get();

    /**
     * Functions to check type conversion and cast when possible under a given filter operation
     * @param ret the casted value
     * @param op the FilterOperation operating on the Literal
     * @return true if cast is successful
     */
    bool as_epoch_date() override { return true; }

    bool as_float_date() override { return true; }

    bool as_var_string(std::string& ret, FilterOperation op) override;
    bool as_float(double& ret, FilterOperation op) override;
    bool as_int(int64_t& ret, FilterOperation op) override;

protected:
    Integral64 m_v;
    std::string m_vstr;  // original string representation if created from string

    explicit Integral(double v);
    explicit Integral(int64_t v);
};
}  // namespace clp_structured

#endif  // CLP_STRUCTURED_SEARCH_INTEGRAL_H