object PStart: TPStart
  Left = 700
  Top = 203
  Anchors = [akLeft, akBottom]
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = #1055#1088#1077#1076#1089#1090#1072#1088#1090
  ClientHeight = 410
  ClientWidth = 500
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  OnActivate = FormActivate
  OnHide = onCloseWin
  DesignSize = (
    500
    410)
  PixelsPerInch = 96
  TextHeight = 13
  object PorogLbl: TLabel
    Left = 401
    Top = 331
    Width = 68
    Height = 16
    Anchors = [akRight, akBottom]
    Caption = #1087#1086#1088#1086#1075'   '#1084#1042
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    ExplicitLeft = 403
    ExplicitTop = 336
  end
  object PreTimeLbl: TLabel
    Left = 384
    Top = 233
    Width = 99
    Height = 16
    Anchors = [akRight, akBottom]
    Caption = #1074#1088#1077#1084#1103' 0     '#1084#1082#1089
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
  end
  object PostTimeLbl: TLabel
    Left = 384
    Top = 282
    Width = 100
    Height = 16
    Anchors = [akRight, akBottom]
    Caption = #1074#1088#1077#1084#1103' '#1061'     '#1084#1082#1089
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object SLenLbl1: TLabel
    Left = 390
    Top = 113
    Width = 31
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1076#1083#1080#1085#1072
  end
  object BckTmLbl3: TLabel
    Left = 476
    Top = 176
    Width = 17
    Height = 16
    Anchors = [akTop, akRight]
    Caption = #1084#1089
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
  end
  object BckTmLbl1: TLabel
    Left = 389
    Top = 170
    Width = 30
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1074#1088#1077#1084#1103
    Visible = False
  end
  object BckTmLbl2: TLabel
    Left = 390
    Top = 183
    Width = 30
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1085#1072#1079#1072#1076
    Visible = False
  end
  object SLenLbl2: TLabel
    Left = 385
    Top = 125
    Width = 40
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1089#1080#1075#1085#1072#1083#1072
    ExplicitLeft = 389
  end
  object SLenLbl3: TLabel
    Left = 476
    Top = 117
    Width = 17
    Height = 16
    Anchors = [akTop, akRight]
    Caption = #1084#1089
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object LenSpntLbl3: TLabel
    Left = 476
    Top = 146
    Width = 17
    Height = 16
    Anchors = [akTop, akRight]
    Caption = #1084#1089
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
  end
  object LenSpntLbl1: TLabel
    Left = 389
    Top = 140
    Width = 31
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1076#1083#1080#1085#1072
    Visible = False
  end
  object LenSpntLbl2: TLabel
    Left = 385
    Top = 153
    Width = 41
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1089#1087#1086#1085#1090#1072#1085
    Visible = False
  end
  object GetASignal: TButton
    Left = 385
    Top = 3
    Width = 90
    Height = 23
    Anchors = [akTop, akRight]
    Caption = #1055#1088#1080#1084#1077#1088' '#1089#1080#1075#1085#1072#1083#1072
    TabOrder = 0
    OnClick = GetASignalClick
  end
  object StandardSgnl: TChart
    Left = 0
    Top = 0
    Width = 379
    Height = 373
    AllowPanning = pmNone
    BackWall.Brush.Color = clWhite
    BackWall.Brush.Style = bsClear
    Title.Text.Strings = (
      #1055#1088#1080#1084#1077#1088' '#1089#1080#1075#1085#1072#1083#1072)
    BottomAxis.Automatic = False
    BottomAxis.AutomaticMaximum = False
    BottomAxis.AutomaticMinimum = False
    BottomAxis.ExactDateTime = False
    BottomAxis.Increment = 1.000000000000000000
    BottomAxis.Maximum = 25.000000000000000000
    BottomAxis.Title.Caption = #1074#1088#1077#1084#1103', '#1084#1089
    DepthAxis.Automatic = False
    DepthAxis.AutomaticMaximum = False
    DepthAxis.AutomaticMinimum = False
    DepthAxis.Maximum = 0.169999999999999800
    DepthAxis.Minimum = -0.830000000000000300
    LeftAxis.ExactDateTime = False
    LeftAxis.Increment = 0.100000000000000000
    LeftAxis.Title.Caption = #1085#1072#1087#1088#1103#1078#1077#1085#1080#1077', '#1084#1042
    Legend.Visible = False
    RightAxis.Automatic = False
    RightAxis.AutomaticMaximum = False
    RightAxis.AutomaticMinimum = False
    RightAxis.Labels = False
    RightAxis.Visible = False
    TopAxis.Automatic = False
    TopAxis.AutomaticMaximum = False
    TopAxis.AutomaticMinimum = False
    TopAxis.Visible = False
    View3D = False
    View3DWalls = False
    Color = 15923449
    TabOrder = 1
    Anchors = [akLeft, akTop, akRight, akBottom]
    OnMouseDown = StandardSgnlMouseDown
    object porogLine: TFastLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clRed
      LinePen.Color = clRed
      LinePen.Width = 2
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object preTLine: TFastLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = 10485760
      LinePen.Color = 10485760
      LinePen.Width = 2
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
    object postTLine: TFastLineSeries
      Marks.ArrowLength = 8
      Marks.Visible = False
      SeriesColor = clFuchsia
      LinePen.Color = clFuchsia
      LinePen.Width = 2
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Multiplier = 1.000000000000000000
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Multiplier = 1.000000000000000000
      YValues.Order = loNone
    end
  end
  object StartRec: TButton
    Left = 385
    Top = 30
    Width = 55
    Height = 23
    Anchors = [akTop, akRight]
    Caption = #1057#1090#1072#1088#1090
    TabOrder = 2
    OnClick = StartRecClick
  end
  object CloseWin: TButton
    Left = 440
    Top = 363
    Width = 58
    Height = 45
    Anchors = [akRight, akBottom]
    Caption = #1057#1090#1086#1087
    TabOrder = 3
    OnClick = CloseWinClick
  end
  object PreTSel: TUpDown
    Left = 380
    Top = 212
    Width = 61
    Height = 22
    Anchors = [akRight, akBottom]
    Orientation = udHorizontal
    TabOrder = 4
    Visible = False
    OnClick = PreTSelClick
  end
  object AmpPorogSel: TUpDown
    Left = 398
    Top = 308
    Width = 39
    Height = 24
    Anchors = [akRight, akBottom]
    Min = -10000
    Max = 10000
    TabOrder = 5
    OnClick = AmpPorogSelClick
  end
  object PostTSel: TUpDown
    Left = 380
    Top = 261
    Width = 60
    Height = 22
    Anchors = [akRight, akBottom]
    Orientation = udHorizontal
    Position = 1
    TabOrder = 6
    OnClick = PostTSelClick
  end
  object PreTime: TEdit
    Left = 439
    Top = 212
    Width = 60
    Height = 22
    Anchors = [akRight, akBottom]
    BiDiMode = bdRightToLeft
    Color = clBlue
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBiDiMode = False
    ParentFont = False
    TabOrder = 7
    Text = '0'
    Visible = False
    OnChange = PreTimeChange
    OnKeyPress = CheckForKeyPStartEdits
  end
  object PostTime: TEdit
    Left = 440
    Top = 261
    Width = 60
    Height = 22
    Anchors = [akRight, akBottom]
    BiDiMode = bdRightToLeft
    Color = clFuchsia
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBiDiMode = False
    ParentFont = False
    TabOrder = 8
    Text = '0'
    OnChange = PostTimeChange
    OnKeyPress = CheckForKeyPStartEdits
  end
  object ExpInfo: TButton
    Left = 385
    Top = 58
    Width = 90
    Height = 23
    Anchors = [akTop, akRight]
    Caption = #1080#1085#1092#1086#1088#1084' '#1101#1082#1089#1087
    TabOrder = 9
    OnClick = ExpInfoClick
  end
  object Porog: TEdit
    Left = 437
    Top = 308
    Width = 45
    Height = 24
    Anchors = [akRight, akBottom]
    BiDiMode = bdRightToLeft
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBiDiMode = False
    ParentFont = False
    TabOrder = 10
    Text = '0'
    OnChange = PorogChange
    OnKeyPress = CheckForKeyPPorogEdit
  end
  object SignalLen: TEdit
    Left = 429
    Top = 116
    Width = 40
    Height = 21
    Anchors = [akTop, akRight]
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 11
    Text = '15'
    OnChange = SignalLenChange
    OnKeyPress = CheckForKeyPStartEdits
  end
  object Invert: TCheckBox
    Left = 385
    Top = 91
    Width = 61
    Height = 17
    Anchors = [akTop, akRight]
    Caption = #1080#1085#1074#1077#1088#1090
    TabOrder = 12
  end
  object NulMinus: TCheckBox
    Left = 454
    Top = 91
    Width = 32
    Height = 17
    Anchors = [akTop, akRight]
    Caption = '- 0'
    Checked = True
    State = cbChecked
    TabOrder = 13
  end
  object BackTime: TEdit
    Left = 429
    Top = 175
    Width = 40
    Height = 21
    Anchors = [akTop, akRight]
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 14
    Text = '3'
    Visible = False
    OnChange = BackTimeChange
    OnKeyPress = CheckForKeyPStartEdits
  end
  object NextBlock: TButton
    Left = 381
    Top = 388
    Width = 55
    Height = 20
    Anchors = [akRight, akBottom]
    Caption = #1089#1083#1077#1076'. '#1073#1083#1086#1082
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 15
    Visible = False
    OnClick = NextBlockClick
  end
  object PausSpontan: TButton
    Left = 381
    Top = 363
    Width = 55
    Height = 20
    Anchors = [akRight, akBottom]
    Caption = #1087#1072#1091#1079#1072' '#1089#1087#1085#1090
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    TabOrder = 16
    Visible = False
    OnClick = PausSpontanClick
  end
  object TimeScaleBar: TTrackBar
    Left = 0
    Top = 0
    Width = 380
    Height = 26
    Anchors = [akLeft, akTop, akRight]
    TabOrder = 17
    Visible = False
    OnChange = TimeBarChange
  end
  object TimeBar: TTrackBar
    Left = 0
    Top = 375
    Width = 379
    Height = 33
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 18
    TickMarks = tmTopLeft
    Visible = False
    OnChange = TimeBarChange
  end
  object LenSpont: TEdit
    Left = 430
    Top = 148
    Width = 40
    Height = 21
    Anchors = [akTop, akRight]
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 19
    Text = '100'
    Visible = False
    OnChange = LenSpontChange
    OnKeyPress = CheckForKeyPStartEdits
  end
end
