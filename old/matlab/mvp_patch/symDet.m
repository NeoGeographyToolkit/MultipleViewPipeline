n = 3;
str_a = ['syms '];
str_b = str_a;
vec_a = ['a = ['];
vec_b = ['b = ['];
for k = 1:n
    str_a = [str_a ' a' num2str(k)];
    str_b = [str_b ' b' num2str(k)];
    vec_a = [vec_a ' a' num2str(k)];
    vec_b = [vec_b ' b' num2str(k)];
end
vec_a = [vec_a ']'];
vec_b = [vec_b ']'];
eval([str_a ' real'])
eval([str_b ' real'])
eval(vec_a)
eval(vec_b)
