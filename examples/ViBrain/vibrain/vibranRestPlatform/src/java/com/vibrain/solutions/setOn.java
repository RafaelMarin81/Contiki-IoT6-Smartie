/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.vibrain.solutions;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;

/**
 * REST Web Service
 *
 * @author Hp
 */
@Path("/json/actions")
public class setOn {

        @GET
	@Path("/get")
	@Produces("application/json")
	public String getProductInJSON() {            
             //   EntityOut estado = new EntityOut();
               // estado.setEstado("ON");
                System.out.println("LLamada :)");
               // return estado;
                 return "ON";
	}

}
