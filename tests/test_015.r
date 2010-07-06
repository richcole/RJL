 { sys |

   sys Collection map = { fn | 
     local result = sys Array new ;
     self each { x | result push (fn x) } ;
     return result ;
   }
   
   sys Array mixin ( sys Sequence ) ;

   sys MyObject mixin ( sys Sequence ) ;

   sys Sequence requires ( 
     sys MethodSignature new_with {| name = "each"; arity = 1 |} 
   ) ;

   sys Object mixin = { module |
     module requirements each { requirement |
       requirement check self ;
     }
     parents push mixin ;
   }

   sys Object requires = { requirement | 
     self array_set :requirements requirement ;
   }

   sys Object array_set = { slot value |
     local array = self send slot ;
     if array == nil then
       array = (self assign slot (sys Array new)) ;
     end
     array push value ;
   }

   sys MethodSignature = sys Class new_with {|
     methods = {|
       check = { target | 
         local slot = self get slot ;
         if slot == nil then
           throw (sys MethodMissing new_with 
             {| method = name; on = target; requirement: self |}) ;
         elsif not (slot isBlock) then
           throw (sys ExpectedBlock new_with
             {| method = name; on = target; requirement: self |}) ;
         elsif not (slot arrity == 1) then
           throw (sys ExpectedBlockWithArity new_with
             {| method = name; on = target; requirement: self |}) ;
         end
       }
     } 
   |}
}
