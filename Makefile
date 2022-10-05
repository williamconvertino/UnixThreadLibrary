all: test0 test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11


test0: thread.cc test0.cc libinterrupt.a
	g++ -o app0 thread.cc test0.cc libinterrupt.a -ldl -no-pie
	./app0

test1: thread.cc test1.cc libinterrupt.a
	g++ -o app1 thread.cc test1.cc libinterrupt.a -ldl -no-pie
	./app1

test2: thread.cc test2.cc libinterrupt.a
	g++ -o app2 thread.cc test2.cc libinterrupt.a -ldl -no-pie
	./app2

test3: thread.cc test3.cc libinterrupt.a
	g++ -o app3 thread.cc test3.cc libinterrupt.a -ldl -no-pie
	./app3

test4: thread.cc test4.cc libinterrupt.a
	g++ -o app4 thread.cc test4.cc libinterrupt.a -ldl -no-pie
	./app4

test5: thread.cc test5.cc libinterrupt.a
	g++ -o app5 thread.cc test5.cc libinterrupt.a -ldl -no-pie
	./app5

test6: thread.cc test6.cc libinterrupt.a
	g++ -o app6 thread.cc test6.cc libinterrupt.a -ldl -no-pie
	./app6

test7: thread.cc test7.cc libinterrupt.a
	g++ -o app7 thread.cc test7.cc libinterrupt.a -ldl -no-pie
	./app7

test8: thread.cc test8.cc libinterrupt.a
	g++ -o app8 thread.cc test8.cc libinterrupt.a -ldl -no-pie
	./app8

test9: thread.cc test9.cc libinterrupt.a
	g++ -o app9 thread.cc test9.cc libinterrupt.a -ldl -no-pie
	./app9

test10: thread.cc test10.cc libinterrupt.a
	g++ -o app10 thread.cc test10.cc libinterrupt.a -ldl -no-pie
	./app10

test11: thread.cc test11.cc libinterrupt.a
	g++ -o app11 thread.cc test11.cc libinterrupt.a -ldl -no-pie
	./app11