t <html><head><title>Hora</title></head>
i pg_header.inc
t <h2 align=center><br>Hora</h2>
t <p><font size="2">Esta pagina muestra la fecha y hora por el lcd y por la web.
t  Para actualizar fecha y hora en la web pulsa <b>Actualizar</b>.<br><br>
t  This Form uses a <b>GET</b> method to send data to a Web server.</font></p>
t <form action=hora.cgi method=get name=cgi>
t <input type=hidden value="hora" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Valor</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>Hora</td>
c h 1 <td>%s</td></tr>
t <tr><td><img src=pabb.gif>Fecha</TD>
c h 2 <td>%s</td></tr>
t </font></table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Actualizar" id="sbm">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.
