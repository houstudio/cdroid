with Ada.Sequential_IO;

package body Hello_1 is
   package String_IO is new Ada.Sequential_IO(Character); -- Instantiate a generic with Character.
   Test_Ada_EOL_File : String_IO.File_Type;
   A_String : String := "hello, world";
   
   procedure Hello is
   begin
      String_IO.Create(Test_Ada_EOL_File, String_IO.Out_File, "hello.out");
      for A_Character of A_String loop
	 String_IO.Write(Test_Ada_EOL_File, A_Character);
      end loop;
      String_IO.Close(Test_Ada_EOL_File);
   end;
end Hello_1;
