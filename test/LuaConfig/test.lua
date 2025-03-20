
toptable = {

	intlist = { 1, 2, },
	strlist = { "s1", "s2", "s3", "s4", "s5", "s6", "s7", },
	intlist1 = { a=1, b=2, },

	mixedlist = { 123, 3.1415, "teststr", false },

	my_global = 1,

	MyTable1 = {
		name = "scott",
		size = 10,
		rate = 1.234,
		ilist = { 11,22,33,44,55 },
		err  = false,
		MyTable1a = {
			inner_name = "scott",
			inner_size = 10,
			inner_rate = 1.234,
			inner_err  = false,
		},
	},

	MyTable2 = {
		str = "test2",
		bl  = true,
	},

	my_global2 = "aaaa",
}

table1 = {
	iv =1234,
	tt = toptable,
}

table2 = {
	toptable, table1,
}

return table2;
