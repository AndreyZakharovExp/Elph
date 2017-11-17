object ProtoBuild: TProtoBuild
  Left = 0
  Top = 0
  Anchors = [akLeft, akBottom]
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = #1047#1072#1076#1072#1085#1080#1077' '#1087#1088#1086#1090#1086#1082#1086#1083#1072' '#1089#1090#1080#1084#1091#1083#1103#1094#1080#1080
  ClientHeight = 178
  ClientWidth = 564
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnHide = EndOfProtoBuild
  DesignSize = (
    564
    178)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 51
    Width = 138
    Height = 13
    Caption = #1089#1090#1080#1084#1091#1083#1086#1074'  '#1089'   '#1087#1077#1088#1080#1086#1076#1086#1084' ('#1084#1089')'
    Visible = False
  end
  object Label2: TLabel
    Left = 78
    Top = 51
    Width = 51
    Height = 13
    Caption = #1087#1072#1091#1079#1072' ('#1084#1089')'
    Visible = False
  end
  object STLbl: TLabel
    Left = 468
    Top = 23
    Width = 91
    Height = 16
    Anchors = [akTop, akRight]
    Caption = #1042#1088#1077#1084#1103' '#1089#1090#1072#1088#1090#1072
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    ExplicitLeft = 475
  end
  object HLbl: TLabel
    Left = 468
    Top = 54
    Width = 29
    Height = 16
    Anchors = [akTop, akRight]
    Caption = #1095#1072#1089#1099
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    ExplicitLeft = 475
  end
  object MLbl: TLabel
    Left = 454
    Top = 97
    Width = 43
    Height = 16
    Anchors = [akTop, akRight]
    Caption = #1084#1080#1085#1091#1090#1099
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    ExplicitLeft = 461
  end
  object SLbl: TLabel
    Left = 449
    Top = 140
    Width = 48
    Height = 16
    Anchors = [akTop, akRight]
    Caption = #1089#1077#1082#1091#1085#1076#1099
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    ExplicitLeft = 456
  end
  object Label4: TLabel
    Left = 175
    Top = 52
    Width = 39
    Height = 13
    Caption = #1074' '#1094#1080#1082#1083#1077
    Visible = False
  end
  object Label3: TLabel
    Left = 168
    Top = 40
    Width = 54
    Height = 13
    Caption = #1087#1086#1074#1090#1086#1088#1103#1090#1100
    Visible = False
  end
  object InstructBox: TScrollBox
    Left = 0
    Top = 107
    Width = 436
    Height = 71
    HorzScrollBar.Smooth = True
    Anchors = [akLeft, akTop, akRight]
    DragKind = dkDock
    TabOrder = 0
  end
  object StimBlock: TButton
    Left = 8
    Top = 8
    Width = 56
    Height = 25
    Hint = #1091#1089#1090#1072#1085#1086#1074#1082#1072' '#1082#1086#1083#1080#1095#1077#1089#1090#1074#1086' '#1080#1084#1087#1091#1083#1100#1089#1086#1074' '#1074' '#1073#1083#1086#1082#1077' '#1080' '#1087#1077#1088#1080#1086#1076' '#1089#1083#1077#1076#1086#1074#1072#1085#1080#1103
    Caption = #1089#1090#1080#1084#1091#1083
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
    OnClick = StimBlockClick
  end
  object Silence: TButton
    Left = 76
    Top = 8
    Width = 65
    Height = 25
    Hint = #1091#1089#1090#1072#1085#1086#1074#1082#1072' '#1087#1072#1091#1079#1099' - '#1087#1077#1088#1077#1088#1099#1074#1072' '#1074' '#1089#1090#1080#1084#1091#1083#1103#1094#1080#1080
    Caption = #1086#1078#1080#1076#1072#1085#1080#1077
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    OnClick = SilenceClick
  end
  object StimsNum: TEdit
    Left = 8
    Top = 66
    Width = 41
    Height = 21
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 3
    Text = '2'
    Visible = False
    OnKeyPress = CheckForKeyOtherEdits
  end
  object Periods: TEdit
    Left = 70
    Top = 66
    Width = 57
    Height = 21
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 4
    Text = '20'
    Visible = False
    OnKeyPress = CheckForKeyOtherEdits
  end
  object Repeat: TButton
    Left = 172
    Top = 8
    Width = 49
    Height = 25
    Hint = #1079#1072#1076#1072#1085#1080#1077' '#1095#1080#1089#1083#1072' '#1087#1086#1074#1090#1086#1088#1077#1085#1080#1081' '#1074#1089#1077#1075#1086' '#1089#1094#1077#1085#1072#1088#1080#1103
    Caption = #1094#1080#1082#1083
    ParentShowHint = False
    ShowHint = True
    TabOrder = 5
    OnClick = RepeatClick
  end
  object repeats: TEdit
    Left = 170
    Top = 66
    Width = 49
    Height = 21
    BiDiMode = bdRightToLeft
    ParentBiDiMode = False
    TabOrder = 6
    Text = '1'
    Visible = False
    OnKeyPress = CheckForKeyOtherEdits
  end
  object DelLast: TButton
    Left = 264
    Top = 8
    Width = 67
    Height = 25
    Hint = #1086#1090#1084#1077#1085#1072' '#1087#1086#1089#1083#1077#1076#1085#1077#1081' '#1091#1089#1090#1072#1085#1086#1074#1082#1080
    Caption = #1086#1090#1084#1077#1085#1080#1090#1100' 1'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 7
    OnClick = DelLastClick
  end
  object AcceptProt: TButton
    Left = 264
    Top = 39
    Width = 67
    Height = 25
    Hint = #1079#1072#1082#1088#1099#1090#1100' '#1080' '#1087#1088#1080#1084#1077#1085#1080#1090#1100' '#1089#1086#1079#1076#1072#1085#1085#1099#1081' '#1087#1088#1086#1090#1086#1082#1086#1083
    Caption = #1087#1088#1080#1084#1077#1085#1080#1090#1100
    ParentShowHint = False
    ShowHint = True
    TabOrder = 8
    OnClick = CloseClicked
  end
  object saveProtocol: TButton
    Left = 264
    Top = 73
    Width = 67
    Height = 27
    Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100
    TabOrder = 9
    OnClick = SaveProtocolClick
  end
  object loadProtocol: TButton
    Left = 337
    Top = 39
    Width = 67
    Height = 25
    Caption = #1047#1072#1075#1088#1091#1079#1080#1090#1100
    TabOrder = 10
    OnClick = LoadProtocolClick
  end
  object CancelAll: TButton
    Left = 337
    Top = 8
    Width = 67
    Height = 25
    Caption = #1086#1090#1084'. '#1074#1089#1105
    TabOrder = 11
    OnClick = CancelAllClick
  end
  object StarTim: TCheckBox
    Left = 337
    Top = 79
    Width = 88
    Height = 17
    Caption = #1042#1088#1077#1084#1103' '#1089#1090#1072#1088#1090#1072
    TabOrder = 12
    OnClick = StarTimClick
  end
  object HEdit: TEdit
    Left = 501
    Top = 51
    Width = 41
    Height = 24
    Anchors = [akTop, akRight]
    BiDiMode = bdRightToLeft
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBiDiMode = False
    ParentFont = False
    TabOrder = 13
    Text = '00'
    OnChange = EditChange
  end
  object HUpDown: TUpDown
    Left = 542
    Top = 49
    Width = 17
    Height = 25
    Anchors = [akTop, akRight]
    Max = 24
    TabOrder = 14
    OnClick = UpDownClick
  end
  object MEdit: TEdit
    Tag = 1
    Left = 501
    Top = 94
    Width = 41
    Height = 24
    Anchors = [akTop, akRight]
    BiDiMode = bdRightToLeft
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBiDiMode = False
    ParentFont = False
    TabOrder = 15
    Text = '00'
    OnChange = EditChange
  end
  object MUpDown: TUpDown
    Tag = 1
    Left = 542
    Top = 92
    Width = 17
    Height = 25
    Anchors = [akTop, akRight]
    Max = 59
    TabOrder = 16
    OnClick = UpDownClick
  end
  object SEdit: TEdit
    Tag = 2
    Left = 501
    Top = 137
    Width = 41
    Height = 24
    Anchors = [akTop, akRight]
    BiDiMode = bdRightToLeft
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentBiDiMode = False
    ParentFont = False
    TabOrder = 17
    Text = '00'
    OnChange = EditChange
  end
  object SUpDown: TUpDown
    Tag = 2
    Left = 542
    Top = 135
    Width = 17
    Height = 25
    Anchors = [akTop, akRight]
    Max = 59
    TabOrder = 18
    OnClick = UpDownClick
  end
  object SaveDialog1: TSaveDialog
    DefaultExt = 'spf'
    Filter = '*.spf|*.spf'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Title = #1057#1086#1093#1088#1072#1085#1077#1085#1080#1077' '#1087#1088#1086#1090#1086#1082#1086#1083#1072
    Left = 134
    Top = 80
  end
  object OpenDialog1: TOpenDialog
    Filter = '*.spf|*.spf'
    Left = 224
    Top = 80
  end
end
