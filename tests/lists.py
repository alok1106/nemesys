a = [1, 2, 3, 4]
print('a has ' + repr(len(a)) + ' items')
for x in a:
  print(repr(x))
else:
  print('done')

print('a[2] is ' + repr(a[2]))
try:
  print('a[5] is ' + repr(a[5]))
except IndexError:
  print('a[5] does not exist')

b = ['a', 'b', 'c']
print('b has ' + repr(len(b)) + ' items')
for y in b:
  print(y)
else:
  print('done')

b.clear()
print('b was cleared; now it has ' + repr(len(b)) + ' items')
for y in b:
  print(y)
else:
  print('done')
