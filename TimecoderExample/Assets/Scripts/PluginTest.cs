using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;

public class PluginTest : MonoBehaviour
{
    [DllImport("Timecoder")]
    private static extern string PrintHello();

    void Start()
    {
        PrintHello();
    }
}
