// Polypheides @feature Polypheides 18/03/2026
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Microsoft.Win32;

public class Launcher : Form {
    private ComboBox dropDown;
    private TextBox flagsBox;
    private string regKey = @"Software\Electronic Arts\EA Games\Command and Conquer Generals Zero Hour";
    private string exePath = "generalszh.exe";
    private string iniPath = @"Data\Language.ini";
    private string iconPath = "GeneralsZH.ico";
    private string imagePath = "launcher.bmp";

    public Launcher() {
        this.Text = "Generals ZH Launcher";
        this.Size = new Size(320, 260); // Initial size
        this.StartPosition = FormStartPosition.CenterScreen;
        this.FormBorderStyle = FormBorderStyle.FixedDialog;
        this.MaximizeBox = false;

        // Try to load user favicon icon
        if (File.Exists(iconPath)) {
            try { this.Icon = new Icon(iconPath); } catch { }
        } else {
            try { this.Icon = Icon.ExtractAssociatedIcon(exePath); } catch { }
        }

        int shift = 0;
        // Try to load a banner image if it exists
        if (File.Exists(imagePath)) {
            try {
                PictureBox pb = new PictureBox();
                pb.Image = Image.FromFile(imagePath);
                pb.SizeMode = PictureBoxSizeMode.AutoSize;
                pb.Location = new Point(0, 0);
                this.Controls.Add(pb);
                shift = pb.Height + 10;
                this.Width = Math.Max(this.Width, pb.Width + 10);
                this.Height += shift;
            } catch { }
        }

        // Language Selection
        Label label = new Label() { Text = "Select Language:", Location = new Point(20, 20 + shift), AutoSize = true };
        this.Controls.Add(label);

        dropDown = new ComboBox() { Location = new Point(20, 45 + shift), Size = new Size(260, 30), DropDownStyle = ComboBoxStyle.DropDownList, Font = new Font("Arial", 10) };
        
        List<string> languages = new List<string>();
        if (File.Exists(iniPath)) {
            string[] lines = File.ReadAllLines(iniPath);
            foreach (string line in lines) {
                string trimmed = line.Trim();
                if (string.IsNullOrEmpty(trimmed) || trimmed.StartsWith(";")) continue;
                Match match = Regex.Match(trimmed, @"^Language\s+([^;]+)$", RegexOptions.IgnoreCase);
                if (match.Success) {
                    string lang = match.Groups[1].Value.Trim();
                    if (!string.IsNullOrEmpty(lang)) languages.Add(lang);
                }
            }
        }
        if (languages.Count == 0) languages.Add("English");
        foreach (string l in languages) dropDown.Items.Add(l);

        // Persistent Custom Flags
        Label flagsLabel = new Label() { Text = "Custom Launch Flags:", Location = new Point(20, 85 + shift), AutoSize = true };
        this.Controls.Add(flagsLabel);

        flagsBox = new TextBox() { Location = new Point(20, 105 + shift), Size = new Size(260, 20), Text = "-win -quickstart -nologo" };
        this.Controls.Add(flagsBox);

        // Load values from registry (C# 5.0 compatible)
        string currentSelection = languages[0];
        RegistryKey key = Registry.CurrentUser.OpenSubKey(regKey);
        if (key != null) {
            object lVal = key.GetValue("Language");
            if (lVal != null) currentSelection = lVal.ToString();
            
            object fVal = key.GetValue("ExtraFlags");
            if (fVal != null) flagsBox.Text = fVal.ToString();
        }
        
        if (!languages.Contains(currentSelection)) currentSelection = languages[0];
        dropDown.SelectedItem = currentSelection;
        this.Controls.Add(dropDown);

        // Launch Button
        Button btn = new Button() { Text = "LAUNCH GAME", Location = new Point(20, 150 + shift), Size = new Size(260, 45), Font = new Font("Arial", 10, FontStyle.Bold), BackColor = Color.FromArgb(255, 128, 0), ForeColor = Color.White };
        btn.Click += delegate(object sender, EventArgs e) {
            RegistryKey newKey = Registry.CurrentUser.CreateSubKey(regKey);
            newKey.SetValue("Language", dropDown.SelectedItem.ToString());
            newKey.SetValue("ExtraFlags", flagsBox.Text);
            
            if (File.Exists(exePath)) {
                Process.Start(new ProcessStartInfo(exePath, flagsBox.Text));
            }
            Application.Exit();
        };
        this.Controls.Add(btn);

        // Final size adjustments
        this.ClientSize = new Size(Math.Max(300, dropDown.Right + 20), btn.Bottom + 20);
    }

    [STAThread]
    static void Main() {
        Application.EnableVisualStyles();
        Application.Run(new Launcher());
    }
}
