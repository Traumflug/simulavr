#define special_output_port (*( (volatile char *)0x20))
#define special_input_port (*( (volatile char *)0x22))


int main(){
   char str[]="\n\n\nHello world!\n\nPress any key(and enter,if input is buffered, i.e. from keyboard):\n\n";
   char str2[]="\n\n\nYou entered: ";
   char *c,in_char;
   for( c=str; c!=str+sizeof(str); ++c)
      special_output_port = *c;
   in_char= special_input_port;
   for( c=str2; c!=str2+sizeof(str2);++c)
      special_output_port = *c;
   special_output_port = in_char;
   special_output_port = '\n';
   special_output_port= '\n';
   }
