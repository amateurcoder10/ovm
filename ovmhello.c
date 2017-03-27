#include "ovm.c"
#pragma pack (1)
// Begin String definitions
struct mystring { _VTABLE_REF; char *chars; int length;  }; 
typedef struct mystring *HString;

static struct vtable *String_vt;
static struct object *String;

// String>>#new:
static struct object *String_newp(struct closure *cls, struct object *self, char *chars)
{
	HString clone = (HString)send(vtof(self), s_vtallocate, sizeof(struct mystring));
	
	clone->length = strlen(chars);
	clone->chars  = strdup(chars);
	return (struct object *)clone;
}

// String>>#length
static struct object *String_length(struct closure *cls, HString self) { return i2oop(self->length); }

// String>>#print
static struct object * String_print(struct closure * cls, HString self)
{
	int i;

	for (i = 0; i < self->length; i++)
		putchar(self->chars[i]);
	return (struct object * )self;
}

//String>>append
static struct object *String_append(struct closure *cls, HString self, HString string2)
{	HString result=(HString)send(vtof(self),s_vtallocate,sizeof(struct mystring));
	result->length=self->length+string2->length;
	result->chars=strdup(self->chars);
	for(int i=0;i<string2->length;i++)
		result->chars[self->length+i]=string2->chars[i];
	return (struct object *)result;

}

//String>>sizeInmemory

static struct object *String_sim(struct closure *cls,HString self)  

{//printf("%lu %lu\n",sizeof(*self),sizeof(struct mystring)); 
return i2oop(sizeof(*self));}


// ------------------------ Begin Array definitions
struct array { _VTABLE_REF; int length; struct object **contents; };
typedef struct array *HArray;

static struct vtable *Array_vt;
static struct object *Array;

//Array>>#new:
static struct object *Array_newp(struct closure *cls, struct object *self, int length)
{
	HArray clone = (HArray)send(vtof(self), s_vtallocate, sizeof(struct array));

	clone->length   = length;
	clone->contents = (struct object **)calloc(clone->length, sizeof(struct object *));
	assert(clone->contents);
	return (struct object *)clone;
}

//Array>>#length
static struct object *Array_length(struct closure * cls, HArray self) { return i2oop(self->length); }

//Array>>#at:
static struct object *Array_at(struct closure *cls, HArray self, int ix)
{
	// index starts at 1
	if (0 < ix && ix <= self->length)
		return self->contents[ix-1];
	return 0;
}

//Array>>#at:put:
static struct object *Array_atput(struct closure *cls, HArray self, int ix, struct object *rval)
{
	// index starts at 1
	if (0 < ix && ix <= self->length)
		return self->contents[ix-1] = rval;
	return rval;
}

//array>>#sizeinmemory
static struct object *Array_sim(struct closure *cls,HArray self)
{//printf("%lu %lu\n",sizeof(self),sizeof(struct array));
 return i2oop(sizeof(*self));}


static struct symbol *s_at;
static struct symbol *s_atput;
static struct symbol *s_append;
static struct symbol *s_sim;

int main(int argc, char *argv[])
{
	init_ovm();
	
	s_append= (typeof(s_append))send(Symbol,s_newp,"append:");
	s_sim= (typeof(s_append))send(Symbol,s_newp,"sizeinmem:");

	s_at    = (typeof(s_at))   send(Symbol, s_newp, "at:");//newp is already mapped to symbol_newp;send a message with symbol as receiver,selector as s_newp and args as at;its oop is returned
	s_atput = (typeof(s_atput))send(Symbol, s_newp, "at:put:");

	printf("Testing String\n");//creating an object called string which needs two things:a vtable and obj alloc
	String_vt = (typeof(String_vt))send(Object_vt, s_vtdelegate, "String");//send with obj as rx,delegate as the message and String as arg meaning string vt should point to obj_vt
	String    = (typeof(String))send((struct object *)String_vt, s_vtallocate, 0);//send to string_vt a message asking it to allocate a string object

	assert(vtof(String) == String_vt);
	//add methods to the string object
	send(String_vt, s_vtadd_method, s_newp,   (method_t)String_newp);//message to string vt add method is the selector its arguments are method name and method itself
	send(String_vt, s_vtadd_method, s_length, (method_t)String_length);
	send(String_vt, s_vtadd_method, s_print,  (method_t)String_print);
	send(String_vt,s_vtadd_method,s_append,   (method_t)String_append);
	send(String_vt,s_vtadd_method,s_sim,   (method_t)String_sim);

	struct object *greet = send(String, s_newp, "Object Machine v1.0\n");//send a message to string object with selector new and arg as the phrase;send returns an object pointing to the appropriate method with receiver selector and argumenets;basically a pointer to the message in its entirety
	struct object *h     = send(String, s_newp, "hello");
	struct object *sp    = send(String, s_newp, " ");
	struct object *w     = send(String, s_newp, "world");
	struct object *nl    = send(String, s_newp, "\n");

	send(greet, s_print);//now a print message is sent to the object called greet ;send returns a closure which is a binding of object greet and message print along with greet itself to s_print which then prints it
	printf("hello length %d\n", oop2i(send(h, s_length)));

	send(h,  s_print);
	send(sp, s_print);
	send(w,  s_print);
	send(nl, s_print);

	printf("Testing Array\n");
	Array_vt  = (typeof(Array_vt)) send(Object_vt, s_vtdelegate, "Array");
	Array     = (typeof(Array))    send((struct object *)Array_vt,  s_vtallocate, 0);

	assert(vtof(Array) == Array_vt);

	send(Array_vt,  s_vtadd_method, s_newp,   (method_t)Array_newp);
	send(Array_vt,  s_vtadd_method, s_length, (method_t)Array_length);
	send(Array_vt,  s_vtadd_method, s_at,     (method_t)Array_at);
	send(Array_vt,  s_vtadd_method, s_atput,  (method_t)Array_atput);
	send(Array_vt,  s_vtadd_method, s_sim,     (method_t)Array_sim);

	struct object *line = send(Array, s_newp, 4);
	
	printf("array elements %d\n", oop2i(send(line, s_length)));
	send(line, s_atput, 1, h);
	send(line, s_atput, 2, sp);
	send(line, s_atput, 3, w); send(line, s_atput, 4, nl);
	for (int i = 1; i <= 4; i++)
		send(send(line, s_at, i), s_print);
	printf("\n\n");
	printf("testing string method append:\n");
	printf("appending hello and space:\n");
	struct object *hs=send(h,s_append,sp);
	send(hs,s_print);
	printf("\n");
	printf("appending world:\n");
	struct object *hw=send(hs,s_append,w);
	send(hw,s_print);
	printf("\n");
	printf("appending newline:the final string is\n");
	struct object *hn=send(hw,s_append,nl);
	send(hn,s_print);
	
	printf("\n\n");
	printf("Testing new method sizeInMemory for string:\n");
	printf("hello sizeinmemory %d\n", oop2i(send(h, s_sim)));
	printf("world sizeinmemory %d\n", oop2i(send(w, s_sim)));
	struct object *hw2=send(h,s_append,w);
	printf("hello world sizeinmemory %d\n", oop2i(send(hw2, s_sim)));
	
	printf("\n\n");
	printf("Testing new method sizeInMemory for array:\n");
	printf("array line=[1 2 3 4] sizeinmemory %d\n", oop2i(send(line, s_sim)));

	return 0;
}
