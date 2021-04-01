/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mycompany.vibrain;

import com.mycompany.vibrain.utils.Operaciones;
import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;

/**
 *
 * @author Hp
 */
@Path("/json/actions")
public class EstadosDispositivos {

    @GET
    @Path("/on")
    @Produces("application/json")
    public EstadoEntity encienteLampara() {

        EstadoEntity estado = new EstadoEntity();
        System.out.println("LLamada a encender");
        estado.setEstado(Operaciones.setltOn());
        System.out.println("Fin llamar encender");
        return estado;
    }

    @GET
    @Path("/off")
    @Produces("application/json")
    public EstadoEntity apagaLampar() {
        EstadoEntity estado = new EstadoEntity();
        System.out.println("LLamada a apagar");
        estado.setEstado(Operaciones.setltOff());
        System.out.println("Fin llamar apagar");
        return estado;
    }

    @GET
    @Path("/dimmax")
    @Produces("application/json")
    public String dimMax() {
        System.out.println("LLamada a dim 255");
        Operaciones.setDim("255");
        System.out.println("Fin llamar dim m255");
        return "OK";
    }

    @GET
    @Path("/dim75")
    @Produces("application/json")
    public String dim75() {
        System.out.println("LLamada a dim75");
        Operaciones.setDim("191.25");
        System.out.println("Fin llamar dim75");
        return "OK";
    }

    @GET
    @Path("/dimless")
    @Produces("application/json")
    public String dimless() {
        System.out.println("LLamada a dimless");
        Operaciones.setDim("-25.5");
        System.out.println("Fin llamar dimless");
        return "OK";
    }

    @GET
    @Path("/dimmore")
    @Produces("application/json")
    public String dimmore() {
        System.out.println("LLamada a dimmore");
        Operaciones.setDim("25.25");
        System.out.println("Fin llamar dimmore");
        return "OK";
    }
    
    @GET
    @Path("/dimmin")
    @Produces("application/json")
    public String dimmin() {
        System.out.println("LLamada a dimmin");
        Operaciones.setDim("-255");
        System.out.println("Fin llamar dimmin");
        return "OK";
    }
}
