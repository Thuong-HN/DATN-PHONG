using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;


// Declare the delegate prototype to send data back to the form
delegate void AddMessage(string sNewMessage);
delegate void SetTextCallback(string text);
namespace DATN_PHONG_APP
{
    public partial class DATN : Form
    {
        
        private Socket m_sock;                      // Server connection
        private byte[] m_byBuff = new byte[1024];    // Recieved data buffer
        private event AddMessage m_AddMessage;              // Add Message Event handler for Form
        private float nhietdo = 20 , temp_push, temp_down;
        private int nhietdo_push;
        private bool manual_stt = false, stt_push = false, stt_down = false, stt_set = false;
       
        public DATN()
        {
            InitializeComponent();
            
            connected();
            // Add Message Event handler for Form decoupling from input thread
            m_AddMessage = new AddMessage(OnAddMessage);

        }
        private void FormMain_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (m_sock != null && m_sock.Connected)
            {
                m_sock.Shutdown(SocketShutdown.Both);
                m_sock.Close();
            }
        }

        private void Set_Domovan(string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.lblDomovan.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(Set_Domovan);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.lblDomovan.Text = text;
            }
        }
        private void Set_Chedo(string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.lblChedo.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(Set_Chedo);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.lblChedo.Text = text;
            }
        }
        private void Set_Nhietdo(string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.lblNhietdo.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(Set_Nhietdo);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.lblNhietdo.Text = text;
            }
        }
        private void Set_Doam(string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.lblDoam.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(Set_Doam);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.lblDoam.Text = text;
            }
        }
        private void Set_temp_default(string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.lbl_nhietdothucte.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(Set_temp_default);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.lbl_nhietdothucte.Text = text;
           
            }
        }
        private void Set_nhietdo_gia_lap (string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.nhietdo_gialap.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(Set_nhietdo_gia_lap);
                this.Invoke(d, new object[] { text });
            }
            else
            {
                this.nhietdo_gialap.Text = text;
            }
        }
        // *********************** CONNECTING... *********************************
        private void connected()
        {
            Cursor cursor = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;
            try
            {
                // Close the socket if it is still open
                if (m_sock != null && m_sock.Connected)
                {
                    m_sock.Shutdown(SocketShutdown.Both);
                    System.Threading.Thread.Sleep(10);
                    m_sock.Close();
                }

                // Create the socket object
                m_sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                // Define the Server address and port
                IPEndPoint epServer = new IPEndPoint(IPAddress.Parse("192.168.1.184"), 80); //192.168.1.184 => vi => phong 172.20.10.184

                // Connect to the server blocking method and setup callback for recieved data
                // m_sock.Connect( epServer );
                // SetupRecieveCallback( m_sock );

                // Connect to server non-Blocking method
                m_sock.Blocking = false;
                AsyncCallback onconnect = new AsyncCallback(OnConnect);
                m_sock.BeginConnect(epServer, onconnect, m_sock);
                
            }
            catch (Exception)
            {
                //MessageBox.Show(this, ex.Message, "Server Connect failed!");
                MessageBox.Show("Lỗi kết nối đến máy chủ"); pic_connect.Image = Properties.Resources.notconnected;
            }
            Cursor.Current = cursor;
        }
        public void OnConnect(IAsyncResult ar)
        {
            // Socket was the passed in object
            Socket sock = (Socket)ar.AsyncState;

            // Check if we were sucessfull
            try
            {
                //sock.EndConnect( ar );
                if (sock.Connected)
                {
                    SetupRecieveCallback(sock);
                    //senData("Connected");
                    pic_connect.Image = Properties.Resources.connected;
                }
                    
                else {
                    //MessageBox.Show(this, "Unable to connect to remote machine", "Connect Failed!");
                    MessageBox.Show("Lỗi kết nối đến máy chủ"); pic_connect.Image = Properties.Resources.notconnected;
                }
                    
            }
            catch (Exception)
            {
                //MessageBox.Show(this, ex.Message, "Unusual error during Connect!");
                MessageBox.Show("Lỗi kết nối đến máy chủ");
            }
        }
        public void OnRecievedData(IAsyncResult ar)
        {
            // Socket was the passed in object
            Socket sock = (Socket)ar.AsyncState;

            // Check if we got any data
            try
            {
                int nBytesRec = sock.EndReceive(ar);
                if (nBytesRec > 0)
                {
                    // Wrote the data to the List
                    string sRecieved = Encoding.ASCII.GetString(m_byBuff, 0, nBytesRec);

                    // WARNING : The following line is NOT thread safe. Invoke is
                    // m_lbRecievedData.Items.Add( sRecieved );

                    Invoke(m_AddMessage, new string[] { sRecieved });

                    // If the connection is still usable restablish the callback
                    SetupRecieveCallback(sock);
                }
                else
                {
                    // If no data was recieved then the connection is probably dead
                    //Console.WriteLine("Client {0}, disconnected", sock.RemoteEndPoint);
                    sock.Shutdown(SocketShutdown.Both);
                    sock.Close();
                }
            }
            catch (InvalidOperationException exc)
            {
                //MessageBox.Show("Không nhận được dữ liệu");
            }

            catch (Exception exception)
            {
                //MessageBox.Show("Không nhận được dữ liệu");
            }
        }

        // *********************** RECV DATA *********************************
        
        public void OnAddMessage(string sMessage)
        {
            
            try
            {
                string[] tokens = sMessage.Split(',');
                
                if (tokens[0].Equals("sendFlash"))
                {
                    
                    Set_Domovan(tokens[1]);
                    Set_Chedo(tokens[2]);
                    Set_temp_default(tokens[3]);
                    Set_nhietdo_gia_lap(tokens[4]);
                    if (tokens[1] == "0%")
                    {
                        chk_0.Checked = true; showControl.Value = 0; pic_domovan.Image = Properties.Resources.van0;
                        chk_25.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[1] == "25%")
                    {
                        chk_25.Checked = true; showControl.Value = 25; pic_domovan.Image = Properties.Resources.van25;
                        chk_0.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[1] == "50%")
                    {
                        chk_50.Checked = true; showControl.Value = 50; pic_domovan.Image = Properties.Resources.van50;
                        chk_25.Checked = false; chk_0.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[1] == "75%")
                    {
                        chk_75.Checked = true; showControl.Value = 75; pic_domovan.Image = Properties.Resources.van75;
                        chk_25.Checked = false; chk_50.Checked = false; chk_0.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[1] == "100%")
                    {
                        chk_100.Checked = true; showControl.Value = 100; pic_domovan.Image = Properties.Resources.van100;
                        chk_25.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_0.Checked = false;
                    }


                    if (tokens[2] == "auto")
                    {
                        chk_auto.Checked = true; manual_stt = false;
                        chk_manual.Checked = false; 
                        chk_0.Enabled = false; chk_25.Enabled = false; chk_50.Enabled = false; chk_75.Enabled = false; chk_100.Enabled = false;
                        nhietdo_gialap.Enabled = true;gui.Enabled = true;

                    }
                    if (tokens[2] == "manual")
                    {
                        chk_manual.Checked = true; manual_stt = true;
                        chk_auto.Checked = false; 
                        chk_0.Enabled = true; chk_25.Enabled = true; chk_50.Enabled = true; chk_75.Enabled = true; chk_100.Enabled = true;
                        nhietdo_gialap.Enabled = false; gui.Enabled = false;
                    }

                }
                else
                {
                    
                    //Set_Chedo(tokens[2]);
                    nhietdo = float.Parse(tokens[1]);
                    nhietdo_push = (int)nhietdo;
                    Set_Nhietdo(tokens[1]);
                    Set_Doam(tokens[2]);
                    Set_Domovan(tokens[3]);
                    Set_Chedo(tokens[4]);
                    if (stt_set == false)
                    {
                        Set_nhietdo_gia_lap(tokens[1]);
                        stt_set = true;
                    }
                    if (tokens[3] == "0%")
                    {
                        chk_0.Checked = true; showControl.Value = 0; pic_domovan.Image = Properties.Resources.van0;
                        chk_25.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[3] == "25%")
                    {
                        chk_25.Checked = true; showControl.Value = 25; pic_domovan.Image = Properties.Resources.van25;
                        chk_0.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[3] == "50%")
                    {
                        chk_50.Checked = true; showControl.Value = 50; pic_domovan.Image = Properties.Resources.van50;
                        chk_25.Checked = false; chk_0.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[3] == "75%")
                    {
                        chk_75.Checked = true; showControl.Value = 75; pic_domovan.Image = Properties.Resources.van75;
                        chk_25.Checked = false; chk_50.Checked = false; chk_0.Checked = false; chk_100.Checked = false;
                    }
                    if (tokens[3] == "100%")
                    {
                        chk_100.Checked = true; showControl.Value = 100; pic_domovan.Image = Properties.Resources.van100;
                        chk_25.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_0.Checked = false;
                    }

                    if (tokens[4] == "auto")
                    {
                        chk_auto.Checked = true; manual_stt = false;
                        chk_manual.Checked = false;
                        chk_0.Enabled = false; chk_25.Enabled = false; chk_50.Enabled = false; chk_75.Enabled = false; chk_100.Enabled = false;
                        nhietdo_gialap.Enabled = true; gui.Enabled = true;

                    }
                    if (tokens[4] == "manual")
                    {
                        chk_manual.Checked = true; manual_stt = true;
                        chk_auto.Checked = false;
                        chk_0.Enabled = true; chk_25.Enabled = true; chk_50.Enabled = true; chk_75.Enabled = true; chk_100.Enabled = true;
                        nhietdo_gialap.Enabled = false; gui.Enabled = false;
                    }
                }
                
                
            }
            catch (Exception)
            {
                //MessageBox.Show(this, ex.Message, "Setup Recieve Callback failed!");
                //MessageBox.Show("Không thể nhận dữ liệu");
            }

        }
        public void SetupRecieveCallback(Socket sock)
        {
            try
            {
                AsyncCallback recieveData = new AsyncCallback(OnRecievedData);
                sock.BeginReceive(m_byBuff, 0, m_byBuff.Length, SocketFlags.None, recieveData, sock);
            }
            catch (Exception)
            {
                //MessageBox.Show(this, ex.Message, "Setup Recieve Callback failed!");
                MessageBox.Show("Lỗi cấu hình nhận dữ liệu");
            }
        }

        // *********************** SEND *********************************
        private void senData(String text)
        {
            // Check we are connected
            if (m_sock == null || !m_sock.Connected)
            {
                MessageBox.Show(this, "Must be connected to Send a message");
                return;
            }

            // Read the message from the text box and send it
            try
            {
                // Convert to byte array and send.
                Byte[] byteDateLine = Encoding.ASCII.GetBytes(text.ToCharArray());
                m_sock.Send(byteDateLine, byteDateLine.Length, 0);
            }
            catch (Exception)
            {
                //MessageBox.Show(this, ex.Message, "Send Message Failed!");
                MessageBox.Show("Lỗi gửi dữ liệu");
            }
        }



        private void timer1_Tick(object sender, EventArgs e)
        {
            //chart1.Series["ĐIỆN NĂNG"].XValueMember += kwh;
        }



        private void btnConnect_Click_1(object sender, EventArgs e)
        {
            if (m_sock != null && m_sock.Connected)
            {
                m_sock.Shutdown(SocketShutdown.Both);
                m_sock.Close();
            }
            connected();
        }

        private void btnExit_Click_1(object sender, EventArgs e)
        {
            if (m_sock != null && m_sock.Connected)
            {
                m_sock.Shutdown(SocketShutdown.Both);
                m_sock.Close();
            }

            Application.Exit();
        }






        //**************************** CHECKBOX *****************************
        
        private void chk_manual_MouseClick(object sender, MouseEventArgs e)
        {
            if (chk_manual.Checked)
            {
                senData("manualMode***"); manual_stt = true;
                chk_auto.Checked = false; 
                chk_0.Enabled = true; chk_25.Enabled = true; chk_50.Enabled = true; chk_75.Enabled = true; chk_100.Enabled = true;
                
            }
            
        }

        private void chk_auto_MouseClick(object sender, MouseEventArgs e)
        {
            if (chk_auto.Checked)
            {
                senData("autoMode***"); manual_stt = false;
                chk_manual.Checked = false;
                chk_0.Enabled = false; chk_25.Enabled = false; chk_50.Enabled = false; chk_75.Enabled = false; chk_100.Enabled = false;
                
            }
        }
        private void chk_0_MouseClick(object sender, MouseEventArgs e)
        {
            showControl.Value = 0; Set_Domovan("0%");
            senData("0.0%Control***"); pic_domovan.Image = Properties.Resources.van0;
            chk_25.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
        }
        private void chk_25_MouseClick(object sender, MouseEventArgs e)
        {
            if (chk_25.Checked && chk_manual.Checked)
            {
                showControl.Value = 25; Set_Domovan("25%");
                senData("25%Control***"); pic_domovan.Image = Properties.Resources.van25;
                chk_0.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
            }
        }

        private void chk_50_MouseClick(object sender, MouseEventArgs e)
        {
            if (chk_50.Checked && chk_manual.Checked)
            {
                showControl.Value = 50; Set_Domovan("50%");
                senData("50%Control***"); pic_domovan.Image = Properties.Resources.van50;
                chk_25.Checked = false; chk_0.Checked = false; chk_75.Checked = false; chk_100.Checked = false;
            }
        }

        private void gui_Click(object sender, EventArgs e)
        {
            senData("set"+nhietdo_gialap.Text);
        }

        private void chk_75_MouseClick(object sender, MouseEventArgs e)
        {
            if (chk_75.Checked && chk_manual.Checked)
            {
                showControl.Value = 75; Set_Domovan("75%");
                senData("75%Control***"); pic_domovan.Image = Properties.Resources.van75;
                chk_25.Checked = false; chk_50.Checked = false; chk_0.Checked = false; chk_100.Checked = false;
            }
        }

        private void chk_100_MouseClick(object sender, MouseEventArgs e)
        {
            if (chk_100.Checked && chk_manual.Checked)
            {
                showControl.Value = 100; Set_Domovan("100%");
                senData("100%Control***"); pic_domovan.Image = Properties.Resources.van100;
                chk_25.Checked = false; chk_50.Checked = false; chk_75.Checked = false; chk_0.Checked = false;
            }
        }

        
        private void btn_up_Click(object sender, EventArgs e)
        {
            if (manual_stt == false)
            {
                lbl_nhietdothucte.Text = (Convert.ToInt32(lbl_nhietdothucte.Text) + 1).ToString();
                senData("temp"+lbl_nhietdothucte.Text);
            }

        }

        
        private void btn_down_Click(object sender, EventArgs e)
        {
            if (manual_stt == false)
            {
                lbl_nhietdothucte.Text = (Convert.ToInt32(lbl_nhietdothucte.Text) - 1).ToString();
                senData("temp" + lbl_nhietdothucte.Text);
            }

        }

        
    }

}

