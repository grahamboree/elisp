from __future__ import division
# from sets import Set
import re
import sys
import StringIO


class Procedure(object):
    """A user-defined Scheme procedure."""

    def __init__(self, parms, exp, env, runtime):
        self.parms = parms
        self.exp = exp
        self.env = env
        self.runtime = runtime

    def __call__(self, *args):
        return self.runtime.eval(self.exp, Env(self.parms, args, self.env))


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

    def __init__(self, parms=(), args=(), outer=None):
        # Bind parm list to corresponding args, or single parm to list of args
        self.outer = outer
        if isinstance(parms, Symbol):
            self.update({parms: list(args)})
        else:
            if len(args) != len(parms):
                raise TypeError('expected %s, given %s, ' % (Runtime.to_string(parms), Runtime.to_string(args)))
            self.update(zip(parms, args))

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


eof_object = Symbol('#<eof-object>')  # Note: uninterned; can't be read

sym = SymbolTable()
_append = sym["append"]
_begin = sym["begin"]
_cons = sym["cons"]
_define = sym["define"]
_definemacro = sym["define-macro"]
_if = sym["if"]
_lambda = sym["lambda"]
_let = sym["let"]
_quasiquote = sym["quasiquote"]
_quote = sym["quote"]
_set = sym["set!"]
_unquote = sym["unquote"]
_unquotesplicing = sym["unquote-splicing"]


class Runtime(object):
    # Reader Macros
    quotes = {
        "'": _quote,
        "`": _quasiquote,
        ",": _unquote,
        ",@": _unquotesplicing
    }

    # --------------- Constructors
    def __init__(self):
        self.global_env = Env()
        self.add_globals(self.global_env)

        self.macro_table = {_let: self.make_let()}

        self.eval(self.parse("""
            (begin
                (define-macro and (lambda args
                    (if (null? args) #t
                        (if (= (length args) 1) (car args)
                            `(if ,(car args) (and ,@(cdr args)) #f)))))
                ;; More macros can also go here
                )
            """))

    def add_globals(self, env):
        """ Adds the standard symbols to the given environment. """
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
            '<': op.lt,
            '<=': op.le,
            '=': op.eq,
            '>': op.gt,
            '>=': op.ge,
            'append': op.add,
            'apply': lambda proc, l: proc(*l),
            'boolean?': lambda x: isinstance(x, bool),
            'call/cc': Runtime.callcc,
            'car': lambda x: x[0],
            'cdr': lambda x: x[1:],
            'close-input-port': lambda p: p.file.close(),
            'close-output-port': lambda p: p.close(),
            'cons': Runtime.cons,
            'display': lambda x, port=sys.stdout: port.write(x if isinstance(x, str) else Runtime.to_string(x)),
            'eof-object?': lambda x: x is eof_object,
            'eq?': op.is_,
            'equal?': op.eq,
            'eval': lambda x: self.eval(self.expand(x)),
            'length': len,
            'list': lambda *x: list(x),
            'list?': lambda x: isinstance(x, list),
            'load': lambda fn: Runtime.load(fn),
            'not': op.not_,
            'null?': lambda x: x == [],
            'open-input-file': open,
            'open-output-file': lambda f: open(f, 'w'),
            'pair?': Runtime.is_pair,
            'port?': lambda x: isinstance(x, file),
            'read': Runtime.read,
            'read-char': Runtime.readchar,
            'symbol?': lambda x: isinstance(x, Symbol),
            'write': lambda x, port=sys.stdout: port.write(Runtime.to_string(x)),
        })
        pass

    # --------------- Eval
    def eval(self, x, env=None):
        """Evaluate an expression in an environment."""
        if env is None:
            env = self.global_env

        while True:
            if isinstance(x, Symbol):        # variable reference
                return env.find(x)[x]
            elif not isinstance(x, list):    # constant literal
                return x
            elif x[0] is _quote:    # (quote exp)
                (_, exp) = x
                return exp
            elif x[0] is _if:        # (if test conseq alt)
                (_, test, conseq, alt) = x
                x = (conseq if self.eval(test, env) else alt)
            elif x[0] is _set:        # (set! var exp)
                (_, var, exp) = x
                env.find(var)[var] = self.eval(exp, env)
                return None
            elif x[0] is _define:    # (define var exp)
                (_, var, exp) = x
                env[var] = self.eval(exp, env)
                return None
            elif x[0] is _lambda:    # (lambda (var*) exp)
                (_, vars, exp) = x
                return Procedure(vars, exp, env, self)
            elif x[0] is _begin:    # (begin exp+)
                for exp in x[1:-1]:
                    self.eval(exp, env)
                x = x[-1]
            else:                        # (proc exp*)
                exps = [self.eval(exp, env) for exp in x]
                proc = exps.pop(0)
                if isinstance(proc, Procedure):
                    x = proc.exp
                    env = Env(proc.parms, exps, proc.env)
                else:
                    return proc(*exps)

    # --------------- Macro Expansion
    def expand(self, x, toplevel=False):
        """Walk tree of x, making optimizations/fixes, and signaling SyntaxError."""
        def expand_quasiquote(x):
            """Expand `x => 'x; `,x => x; `(,@x y) => (append x y) """
            if not Runtime.is_pair(x):
                return [_quote, x]
            self.require(x, x[0] is not _unquotesplicing, "can't splice here")
            if x[0] is _unquote:
                self.require(x, len(x) == 2)
                return x[1]
            elif Runtime.is_pair(x[0]) and x[0][0] is _unquotesplicing:
                self.require(x[0], len(x[0]) == 2)
                return [_append, x[0][1], expand_quasiquote(x[1:])]
            else:
                return [_cons, expand_quasiquote(x[0]), expand_quasiquote(x[1:])]

        self.require(x, x != [])                # () => Error
        if not isinstance(x, list):        # constant => unchanged
            return x
        elif x[0] is _quote:               # (quote exp)
            self.require(x, len(x) == 2)
            return x
        elif x[0] is _if:
            if len(x) == 3:
                x = x + [None]  # (if t c) => (if t c None)
            self.require(x, len(x) == 4)
            return map(self.expand, x)
        elif x[0] is _set:
            self.require(x, len(x) == 3)
            var = x[1]                    # (set! non-var exp) => Error
            self.require(x, isinstance(var, Symbol), "can set! only a symbol")
            return [_set, var, self.expand(x[2])]
        elif x[0] is _define or x[0] is _definemacro:
            self.require(x, len(x) >= 3)
            _def, v, body = x[0], x[1], x[2:]
            if isinstance(v, list) and v:    # (define (f args) body)
                f, args = v[0], v[1:]        # => (define f (lambda (args) body))
                return self.expand([_def, f, [_lambda, args]+body])
            else:
                self.require(x, len(x) == 3)        # (define non-var/list exp) => Error
                self.require(x, isinstance(v, Symbol), "can define only a symbol")
                exp = self.expand(x[2])
                if _def is _definemacro:
                    self.require(x, toplevel, "define-macro only allowed at top level")
                    proc = self.eval(exp)
                    self.require(x, callable(proc), "macro must be a procedure")
                    self.macro_table[v] = proc    # (define-macro v proc)
                    return None              # => None; add v:proc to macro_table
                return [_define, v, exp]
        elif x[0] is _begin:
            if len(x) == 1:
                return None  # (begin) => None
            else:
                return [self.expand(xi, toplevel) for xi in x]
        elif x[0] is _lambda:                    # (lambda (x) e1 e2)
            self.require(x, len(x) >= 3)              # => (lambda (x) (begin e1 e2))
            vars, body = x[1], x[2:]
            self.require(x, (isinstance(vars, list) and all(isinstance(v, Symbol) for v in vars))
                    or isinstance(vars, Symbol), "illegal lambda argument list")
            exp = body[0] if len(body) == 1 else [_begin] + body
            return [_lambda, vars, self.expand(exp)]
        elif x[0] is _quasiquote:                # `x => expand_quasiquote(x)
            self.require(x, len(x) == 2)
            return expand_quasiquote(x[1])
        elif isinstance(x[0], Symbol) and x[0] in self.macro_table:
            return self.expand(self.macro_table[x[0]](*x[1:]), toplevel)  # (m arg...)
        else:                                    # => macroexpand if m isa macro
            return map(self.expand, x)                # (f arg...) => expand each

    # --------------- parse, read, and user interaction
    def parse(self, inport):
        """Parse a program: read and expand/error-check it."""
        # Backwards compatibility: given a str, convert it to an InPort
        if isinstance(inport, str):
            inport = InPort(StringIO.StringIO(inport))
        return self.expand(Runtime.read(inport), toplevel=True)

    @staticmethod
    def read(inport):
        """Read a Scheme expression from an input port."""

        def atom(token):
            '''Numbers become numbers; #t and #f are booleans; "..." string; otherwise Symbol.'''
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
                L = []
                while True:
                    token = inport.next_token()
                    if token == ')':
                        return L
                    else:
                        L.append(read_ahead(token))
            elif ')' == token:
                raise SyntaxError('unexpected )')
            elif token in Runtime.quotes:
                return [Runtime.quotes[token], Runtime.read(inport)]
            elif token is eof_object:
                raise SyntaxError('unexpected EOF in list')
            else:
                return atom(token)

        # body of read:
        token1 = inport.next_token()
        return eof_object if token1 is eof_object else read_ahead(token1)

    @staticmethod
    def to_string(x):
        """Convert a Python object back into a Lisp-readable string."""
        if x is True:
            return "#t"
        elif x is False:
            return "#f"
        elif isinstance(x, Symbol):
            return x
        elif isinstance(x, str):
            return '"%s"' % x.encode('string_escape').replace('"', r'\"')
        elif isinstance(x, list):
            return '(' + ' '.join(map(Runtime.to_string, x)) + ')'
        elif isinstance(x, complex):
            return str(x).replace('j', 'i')
        return str(x)

    def load(self, filename):
        """Eval every expression from a file."""
        self.repl(None, InPort(open(filename)), None)

    def repl(self, prompt='lispy> ', inport=InPort(sys.stdin), out=sys.stdout):
        """A prompt-read-eval-print loop."""
        sys.stderr.write("Lispy version 2.0\n")
        while True:
            try:
                if prompt:
                    sys.stderr.write(prompt)
                x = self.parse(inport)
                if x is eof_object:
                    return
                val = self.eval(x)
                if val is not None and out:
                    print >> out, self.to_string(val)
                    sys.stdout.flush()
            except Exception as e:
                print '%s: %s' % (type(e).__name__, e)

    # --------------- Builtin macros
    def make_let(self):
        def let(*args):
            """Let macro"""
            args = list(args)
            x = Runtime.cons(_let, args)
            self.require(x, len(args) > 1)
            bindings, body = args[0], args[1:]
            self.require(x, all(isinstance(b, list) and len(b) == 2 and isinstance(b[0], Symbol)
                    for b in bindings), "illegal binding list")
            vars, vals = zip(*bindings)
            return [[_lambda, list(vars)]+map(self.expand, body)] + map(self.expand, vals)
        return let

    # --------------- Prelude
    @staticmethod
    def is_pair(x):
        """Returns whether the given object a pair"""
        return x != [] and isinstance(x, list)

    @staticmethod
    def cons(x, y):
        """Cons"""
        return [x]+y

    @staticmethod
    def readchar(inport):
        """Read the next character from an input port."""
        if inport.line != '':
            ch, inport.line = inport.line[0], inport.line[1:]
            return ch
        else:
            return inport.file.read(1) or eof_object

    @staticmethod
    def callcc(proc):
        """Call proc with current continuation; escape only"""
        ball = RuntimeWarning("Sorry, can't continue this continuation any longer.")

        def throw(retval):
            ball.retval = retval
            raise ball

        try:
            return proc(throw)
        except RuntimeWarning as w:
            if w is ball:
                return ball.retval
            else:
                raise w

    def require(self, x, predicate, msg="wrong length"):
        """Signal a syntax error if predicate is false."""
        if not predicate:
            raise SyntaxError(Runtime.to_string(x) + ': ' + msg)

if __name__ == '__main__':
    r = Runtime()
    r.repl()
