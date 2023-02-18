// SPDX-License-Identifier: MIT
// Copyright 2023 Edouard Gomez

#ifndef scope_guard_h
#define scope_guard_h

namespace
{

/** Functor call on scope exit helper */
template <typename F> struct scope_guard
{
    /** ctor
     * @param f Lambda or functor to be called when the object will get out of
     * scope
     */
    scope_guard(F &&f) : m_f(std::forward<F>(f))
    {
    }

    /** call the embedded lambda/functor if the guard was not dismissed */
    ~scope_guard()
    {
        if (!m_dismissed)
        {
            m_f();
        }
    }
    void dismiss()
    {
        m_dismissed = true;
    }
    F m_f;
    bool m_dismissed = false;
};

/** Helper to create a scope_guard by perfect forwarding the functor
 * @param f Functor to execute on scope guard destruction
 * @return constructed scope guard
 * */
template <typename F> scope_guard<F> make_scope_guard(F &&f)
{
    return scope_guard<F>(std::forward<F>(f));
};

#define scope_guard_str_join(arg1, arg2) scope_guard_str_join_impl(arg1, arg2)
#define scope_guard_str_join_impl(arg1, arg2) arg1##arg2

/** Easily create a scope guard with a chosen variable name, handy for dimissing
 * the guard when needed */
#define on_scope_guard_named(name, func_or_lambda) auto name = make_scope_guard(func_or_lambda)

/** Easily create a scope guard with an automatic variable name */
#define on_scope_guard(func_or_lambda)                                                                                 \
    auto scope_guard_str_join(scopeGuard, __LINE__) = make_scope_guard(func_or_lambda)
} // namespace

#endif // scope_guard_h