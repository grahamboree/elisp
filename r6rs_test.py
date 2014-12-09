from lispy import * 
import unittest

class r6rs_11_1_tests(unittest.TestCase):
	def test_boolean(self):
		r = SchemeRuntime()
		self.assertEquals(r.run("#t"), True)
		self.assertEquals(r.run("#f"), False)
		
		# Predicate
		self.assertEquals(r.run("(boolean? #f)"), True)
		self.assertEquals(r.run("(boolean? 'a)"), False)
		#self.assertEquals(r.run("(boolean? #\\a)"), False)
		#self.assertEquals(r.run("(boolean? #(1 2))"), False)
		self.assertEquals(r.run("(boolean? '(1 2))"), False)
		self.assertEquals(r.run("(boolean? '())"), False)
		#self.assertEquals(r.run("(boolean? (1 . 2))"), False)
		self.assertEquals(r.run('(boolean? "a")'), False)
		self.assertEquals(r.run("(boolean? (lambda (x y) (+ x y)))"), False)

	def test_symbol(self):
		r = SchemeRuntime()
		self.assertEquals(r.run("'t"), Symbol("t"))
		
		# Predicate
		self.assertEquals(r.run("(symbol? #f)"), False)
		self.assertEquals(r.run("(symbol? 'a)"), True)
		#self.assertEquals(r.run("(symbol? #\\a)"), False)
		#self.assertEquals(r.run("(symbol? #(1 2))"), False)
		self.assertEquals(r.run("(symbol? '(1 2))"), False)
		self.assertEquals(r.run("(symbol? '())"), False)
		#self.assertEquals(r.run("(symbol? (1 . 2))"), False)
		self.assertEquals(r.run('(symbol? "a")'), False)
		self.assertEquals(r.run("(symbol? (lambda (x y) (+ x y)))"), False)

if __name__ == '__main__':
	unittest.main()

