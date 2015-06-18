from __future__ import division
import re
import sys
import StringIO


class Procedure(object):
    """A user-defined Scheme procedure."""

    def __init__(self, params, exp, env, runtime):
        self.params = params
        self.exp = exp
        self.env = env
        self.runtime = runtime

    def __call__(self, *args):
        return self.runtime.eval(self.exp, Env(self.params, args, self.env))


class InPort(object):
    """An input port. Retains a line of chars."""
    tokenizer = r"""\s*(,@|[('`,)]|"(?:[\\].|[^\\"])*"|;.*|[^\s('"`,;)]*)(.*)"""

    def __init__(self, file_handle):
        self.file = file_handle
        self.line = ''

    def next_token(self):
        """Return the next token, reading new text into line buffer if needed."""
        while True:
            if self.line == '':
                self.line = self.file.readline()
            if self.line == '':
                return eof_object
            token, self.line = re.match(InPort.tokenizer, self.line).groups()
            if token != '' and not token.startswith(';'):
                return token


class Env(dict):
    """An environment: a dict of {'var':val} pairs, with an outer Env."""

    def __init__(self, params=(), args=(), outer=None):
        super(Env, self).__init__()
        # Bind parm list to corresponding args, or single parm to list of args
        self.outer = outer
        if isinstance(params, Symbol):
            self.update({params: list(args)})
        else:
            if len(args) != len(params):
                raise TypeError('expected %s, given %s, ' % (to_string(params), to_string(args)))
            self.update(zip(params, args))

    def find(self, var):
        """Find the innermost Env where var appears."""
        if var in self:
            return self
        elif self.outer is None:
            raise LookupError(var)
        else:
            return self.outer.find(var)


class Symbol(str):
    pass


class SymbolTable(object):
    def __init__(self):
        self.symbols = {}

    def __getitem__(self, key):
        """Find or create unique Symbol entry for str s in symbol table."""
        if key not in self.symbols:
            self.symbols[key] = Symbol(key)
        return self.symbols[key]


class Continuation(RuntimeWarning):
    def __init__(self, message):
        self.return_val = None
        super(Continuation, self).__init__(message)


eof_object = Symbol('#:<eof-object>')  # Note: not interned; can't be read

sym = SymbolTable()
_append = sym["append"]
_begin = sym["begin"]
_cons = sym["cons"]
_define = sym["define"]
_define_macro = sym["define-macro"]
_if = sym["if"]
_lambda = sym["lambda"]
_let = sym["let"]
_quasiquote = sym["quasiquote"]
_quote = sym["quote"]
_set = sym["set!"]
_unquote = sym["unquote"]
_unquote_splicing = sym["unquote-splicing"]

# Reader Macros
quotes = {
    "'": _quote,
    "`": _quasiquote,
    ",": _unquote,
    ",@": _unquote_splicing
}


def require(x, predicate, msg="wrong length"):
    """Signal a syntax error if predicate is false."""
    if not predicate:
        raise SyntaxError(to_string(x) + ': ' + msg)


def read(in_port):
    """Read a Scheme expression from an input port."""

    def atom(token):
        """ Numbers become numbers; #t and #f are booleans; "..." string; otherwise Symbol. """
        if token == '#t':
            return True
        elif token == '#f':
            return False
        elif token[0] == '"':
            return token[1:-1].decode('string_escape')
        try:
            return int(token)
        except ValueError:
            try:
                return float(token)
            except ValueError:
                try:
                    return complex(token.replace('i', 'j', 1))
                except ValueError:
                    return sym[token]

    def read_ahead(token):
        if '(' == token:
            l = []
            while True:
                token = in_port.next_token()
                if token == ')':
                    return l
                else:
                    l.append(read_ahead(token))
        elif ')' == token:
            raise SyntaxError('unexpected )')
        elif token in quotes:
            return [quotes[token], read(in_port)]
        elif token is eof_object:
            raise SyntaxError('unexpected EOF in list')
        else:
            return atom(token)

    # body of read:
    token1 = in_port.next_token()
    return eof_object if token1 is eof_object else read_ahead(token1)


def callcc(procedure):
    """ Call procedure with current continuation; escape only """
    ball = Continuation("Sorry, can't continue this continuation any longer.")

    def throw(return_val):
        ball.return_val = return_val
        raise ball

    try:
        return procedure(throw)
    except Continuation as w:
        if w is ball:
            return ball.return_val
        else:
            raise w


def read_char(in_port):
    """ Read the next character from an input port. """
    if in_port.line != '':
        ch, in_port.line = in_port.line[0], in_port.line[1:]
        return ch
    else:
        return in_port.file.read(1) or eof_object


def cons(x, y):
    """ Cons """
    return [x] + y


def is_pair(x):
    """ Returns whether the given object a pair """
    return x != [] and isinstance(x, list)


def to_string(x):
    """ Convert a Python object back into a Lisp-readable string. """
    if x is True:
        return "#t"
    elif x is False:
        return "#f"
    elif isinstance(x, Symbol):
        return x
    elif isinstance(x, str):
        return '"%s"' % x.encode('string_escape').replace('"', r'\"')
    elif isinstance(x, list):
        return '({0})'.format(' '.join(map(to_string, x)))
    elif isinstance(x, complex):
        return str(x).replace('j', 'i')
    return str(x)


class Runtime(object):
    def __init__(self):
        self.global_env = self._create_global_scope()

        self.macro_table = {_let: self._make_let()}

        # prelude
        self.run("""
            (begin
                (define-macro and (lambda args
                    (if (null? args) #t
                        (if (= (length args) 1) (car args)
                            `(if ,(car args) (and ,@(cdr args)) #f)))))
                ;; More macros can also go here
            )
            """)

    def _create_global_scope(self):
        """ Creates a scope and adds the standard symbols. """
        env = Env()

        import math
        import cmath
        import operator as op

        env.update(vars(math))
        env.update(vars(cmath))
        env.update({
            '*': op.mul,
            '+': op.add,
            '-': op.sub,
            '/': op.div,

            '=': op.eq,
            'eq?': op.is_,
            'equal?': op.eq,

            '<': op.lt,
            '<=': op.le,
            '>': op.gt,
            '>=': op.ge,

            'not': op.not_,

            'boolean?': lambda x: isinstance(x, bool),
            'eof-object?': lambda x: x is eof_object,
            'list?': lambda x: isinstance(x, list),
            'null?': lambda x: x == [],
            'pair?': is_pair,
            'port?': lambda x: isinstance(x, file),
            'symbol?': lambda x: isinstance(x, Symbol),

            'call/cc': callcc,

            'read': read,
            'read-char': read_char,

            'append': op.add,
            'cons': cons,
            'car': lambda x: x[0],
            'cdr': lambda x: x[1:],
            'length': len,
            'list': lambda *x: list(x),

            'apply': lambda proc, l: proc(*l),
            'close-input-port': lambda p: p.file.close(),
            'close-output-port': lambda p: p.close(),
            'display': lambda x, port=sys.stdout: port.write(x if isinstance(x, str) else to_string(x)),
            'eval': lambda x: self.eval(self.expand(x)),
            'load': lambda fn: self.load(fn),
            'open-input-file': open,
            'open-output-file': lambda f: open(f, 'w'),
            'write': lambda x, port=sys.stdout: port.write(to_string(x)),
        })
        return env

    def run(self, code):
        """
        :param code: Code to evaluate in this runtime
        :return: The result of evaluating that code
        """
        return self.eval(self.parse(InPort(StringIO.StringIO(code))))

    def eval(self, x, env=None):
        """ Evaluate an expression in an environment. """
        if env is None:
            env = self.global_env

        while True:
            if isinstance(x, Symbol):  # variable reference
                return env.find(x)[x]
            elif not isinstance(x, list):  # constant literal
                return x
            elif x[0] is _quote:  # (quote exp)
                (_, exp) = x
                return exp
            elif x[0] is _if:  # (if test conseq alt)
                (_, test, conseq, alt) = x
                x = (conseq if self.eval(test, env) else alt)
            elif x[0] is _set:  # (set! var exp)
                (_, var, exp) = x
                env.find(var)[var] = self.eval(exp, env)
                return None
            elif x[0] is _define:  # (define var exp)
                (_, var, exp) = x
                env[var] = self.eval(exp, env)
                return None
            elif x[0] is _lambda:  # (lambda (var*) exp)
                (_, params, exp) = x
                return Procedure(params, exp, env, self)
            elif x[0] is _begin:  # (begin exp+)
                for exp in x[1:-1]:
                    self.eval(exp, env)
                x = x[-1]
            else:  # (proc exp*)
                expressions = [self.eval(exp, env) for exp in x]
                procedure = expressions.pop(0)
                if isinstance(procedure, Procedure):
                    x = procedure.exp
                    env = Env(procedure.params, expressions, procedure.env)
                else:
                    return procedure(*expressions)

    def expand(self, x, top_level=False):
        """ Walk tree of x, making optimizations/fixes, and signaling SyntaxError. """

        def expand_quasiquote(expr):
            """ Expand `x => 'x; `,x => x; `(,@x y) => (append x y) """
            if not is_pair(expr):
                return [_quote, expr]
            require(expr, expr[0] is not _unquote_splicing, "can't splice here")
            if expr[0] is _unquote:
                require(expr, len(expr) == 2)
                return expr[1]
            elif is_pair(expr[0]) and expr[0][0] is _unquote_splicing:
                require(expr[0], len(expr[0]) == 2)
                return [_append, expr[0][1], expand_quasiquote(expr[1:])]
            else:
                return [_cons, expand_quasiquote(expr[0]), expand_quasiquote(expr[1:])]

        require(x, x != [])  # () => Error
        if not isinstance(x, list):  # constant => unchanged
            return x
        elif x[0] is _quote:  # (quote exp)
            require(x, len(x) == 2)
            return x
        elif x[0] is _if:
            if len(x) == 3:
                x = x + [None]  # (if t c) => (if t c None)
            require(x, len(x) == 4)
            return map(self.expand, x)
        elif x[0] is _set:
            require(x, len(x) == 3)
            var = x[1]  # (set! non-var exp) => Error
            require(x, isinstance(var, Symbol), "can set! only a symbol")
            return [_set, var, self.expand(x[2])]
        elif x[0] is _define or x[0] is _define_macro:
            require(x, len(x) >= 3)
            _def, v, body = x[0], x[1], x[2:]
            if isinstance(v, list) and v:  # (define (f args) body)
                f, args = v[0], v[1:]  # => (define f (lambda (args) body))
                return self.expand([_def, f, [_lambda, args] + body])
            else:
                require(x, len(x) == 3)  # (define non-var/list exp) => Error
                require(x, isinstance(v, Symbol), "can define only a symbol")
                exp = self.expand(x[2])
                if _def is _define_macro:
                    require(x, top_level, "define-macro only allowed at top level")
                    proc = self.eval(exp)
                    require(x, callable(proc), "macro must be a procedure")
                    self.macro_table[v] = proc  # (define-macro v proc)
                    return None  # => None; add v:proc to macro_table
                return [_define, v, exp]
        elif x[0] is _begin:
            if len(x) == 1:
                return None  # (begin) => None
            else:
                return [self.expand(xi, top_level) for xi in x]
        elif x[0] is _lambda:  # (lambda (x) e1 e2)
            require(x, len(x) >= 3)  # => (lambda (x) (begin e1 e2))
            params, body = x[1], x[2:]
            require(x, (isinstance(params, list) and all(isinstance(v, Symbol) for v in params))
                    or isinstance(params, Symbol), "illegal lambda argument list")
            exp = body[0] if len(body) == 1 else [_begin] + body
            return [_lambda, params, self.expand(exp)]
        elif x[0] is _quasiquote:  # `x => expand_quasiquote(x)
            require(x, len(x) == 2)
            return expand_quasiquote(x[1])
        elif isinstance(x[0], Symbol) and x[0] in self.macro_table:
            return self.expand(self.macro_table[x[0]](*x[1:]), top_level)  # (m arg...)
        else:  # => macro expand if m isa macro
            return map(self.expand, x)  # (f arg...) => expand each

    def parse(self, in_port):
        """ Parse a program: read and expand/error-check it. """
        return self.expand(read(in_port), top_level=True)

    def load(self, filename):
        """ Eval every expression from a file. """
        self.repl(InPort(open(filename)))

    def repl(self, prompt=None, in_port=InPort(sys.stdin), out=sys.stdout):
        """ Read-eval-print loop. """
        while True:
            try:
                if prompt:
                    sys.stderr.write(prompt)
                x = self.parse(in_port)
                if x is eof_object:
                    return
                val = self.eval(x)
                if val is not None and out:
                    print >> out, to_string(val)
                    sys.stdout.flush()
            except Exception as e:
                print '{}: {}'.format(type(e).__name__, e)

    def _make_let(self):
        def let(*args):
            """ Let macro """
            args = list(args)

            x = cons(_let, args)
            require(x, len(args) > 1)

            bindings, body = args[0], args[1:]
            require(x, all(isinstance(b, list) and len(b) == 2 and isinstance(b[0], Symbol)
                           for b in bindings), "illegal binding list")
            names, values = zip(*bindings)
            return [[_lambda, list(names)] + map(self.expand, body)] + map(self.expand, values)

        return let


if __name__ == '__main__':
    sys.stderr.write("Lispy version 2.0\n")
    r = Runtime()
    r.repl(prompt='lispy> ')