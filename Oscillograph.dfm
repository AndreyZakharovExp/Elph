object Oscil: TOscil
  Left = 250
  Top = 200
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'OscilloGraph'
  ClientHeight = 435
  ClientWidth = 550
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  OnHide = onOscClose
  OnResize = FormResize
  DesignSize = (
    550
    435)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 492
    Top = 2
    Width = 29
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1087#1086#1088#1086#1075
    ExplicitLeft = 354
  end
  object Label2: TLabel
    Left = 534
    Top = 21
    Width = 12
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1084#1042
    ExplicitLeft = 396
  end
  object Label3: TLabel
    Left = 498
    Top = 117
    Width = 47
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1080#1085#1074#1077#1088#1089#1080#1103
    ExplicitLeft = 360
  end
  object chanlsLbl: TLabel
    Left = 504
    Top = 166
    Width = 38
    Height = 13
    Anchors = [akTop, akRight]
    Caption = #1082#1072#1085#1072#1083#1099
    ExplicitLeft = 366
  end
  object TxtPanel: TPanel
    Left = 0
    Top = 380
    Width = 468
    Height = 57
    Anchors = [akLeft, akRight, akBottom]
    BevelOuter = bvNone
    TabOrder = 19
    DesignSize = (
      468
      57)
    object MinTmpWinLbl: TLabel
      Left = 47
      Top = 12
      Width = 25
      Height = 16
      Anchors = [akLeft, akBottom]
      Caption = '1 '#1084#1089
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object CrntTmpWinLbl: TLabel
      Left = 327
      Top = 12
      Width = 32
      Height = 16
      Anchors = [akRight, akBottom]
      Caption = '10 '#1084#1089
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object MaxTmpWinLbl: TLabel
      Left = 438
      Top = 12
      Width = 24
      Height = 16
      Anchors = [akRight, akBottom]
      Caption = '10 '#1089
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label4: TLabel
      Left = 305
      Top = 32
      Width = 48
      Height = 13
      Anchors = [akRight, akBottom]
      Caption = #1080#1084#1087' '#1074' '#1084#1080#1085
      ExplicitLeft = 214
    end
    object Label5: TLabel
      Left = 177
      Top = 32
      Width = 59
      Height = 13
      Anchors = [akRight, akBottom]
      Caption = #1089#1090#1080#1084#1091#1083#1103#1094#1080#1103
      ExplicitLeft = 86
    end
  end
  object timeScale: TTrackBar
    Left = 24
    Top = 361
    Width = 450
    Height = 34
    Anchors = [akLeft, akRight, akBottom]
    Max = 10000
    Min = 50
    PageSize = 10
    Frequency = 200
    Position = 50
    TabOrder = 1
    TickMarks = tmTopLeft
    OnChange = timeScaleChange
  end
  object NulShift: TTrackBar
    Left = 456
    Top = 51
    Width = 38
    Height = 307
    Anchors = [akTop, akRight, akBottom]
    Max = 10000
    Min = -10000
    Orientation = trVertical
    PageSize = 10
    Frequency = 1000
    TabOrder = 0
    TickMarks = tmTopLeft
    OnChange = NulShiftChange
  end
  object vScale: TTrackBar
    Left = 0
    Top = 8
    Width = 26
    Height = 366
    Anchors = [akLeft, akTop, akBottom]
    Max = 1000
    Min = 1
    Orientation = trVertical
    PageSize = 10
    Frequency = 50
    Position = 100
    TabOrder = 2
    OnChange = vScaleChange
  end
  object acceptParamtr: TButton
    Left = 479
    Top = 401
    Width = 66
    Height = 26
    Anchors = [akRight, akBottom]
    Caption = #1055#1088#1080#1084#1077#1085#1080#1090#1100
    TabOrder = 3
    OnClick = acceptParamtrClick
  end
  object Alternating: TRadioButton
    Left = 509
    Top = 61
    Width = 30
    Height = 16
    Anchors = [akTop, akRight]
    Caption = '~'
    Checked = True
    TabOrder = 4
    TabStop = True
    OnClick = AlternatingClick
  end
  object ConstCompon: TRadioButton
    Left = 509
    Top = 86
    Width = 30
    Height = 17
    Anchors = [akTop, akRight]
    Caption = '--'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsUnderline]
    ParentFont = False
    TabOrder = 5
    OnClick = ConstComponClick
  end
  object invert: TCheckBox
    Left = 512
    Top = 134
    Width = 18
    Height = 18
    Anchors = [akTop, akRight]
    TabOrder = 6
    OnClick = invertClick
  end
  object checkChann0: TCheckBox
    Left = 506
    Top = 180
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '1'
    TabOrder = 7
  end
  object checkChann1: TCheckBox
    Left = 506
    Top = 195
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '2'
    TabOrder = 8
  end
  object checkChann2: TCheckBox
    Left = 506
    Top = 210
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '3'
    TabOrder = 9
  end
  object ImpPerMin: TCSpinEdit
    Left = 242
    Top = 408
    Width = 57
    Height = 24
    Anchors = [akRight, akBottom]
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Tahoma'
    Font.Style = []
    MaxValue = 600
    ParentFont = False
    TabOrder = 10
    OnChange = ImpPerMinChange
    OnKeyPress = RightKeyPress1
  end
  object OneImpulse: TButton
    Left = 366
    Top = 407
    Width = 68
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '1 '#1080#1084#1087#1091#1083#1100#1089
    TabOrder = 11
    OnClick = OneImpulseClick
  end
  object ContRec: TButton
    Left = 481
    Top = 365
    Width = 62
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = #1047#1072#1087#1080#1089#1100
    TabOrder = 12
    OnClick = ContRecClick
  end
  object checkChann3: TCheckBox
    Left = 506
    Top = 225
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '4'
    TabOrder = 13
  end
  object checkChann4: TCheckBox
    Left = 506
    Top = 240
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '5'
    TabOrder = 14
  end
  object Porog: TCSpinEdit
    Left = 470
    Top = 17
    Width = 61
    Height = 24
    Anchors = [akTop, akRight]
    AutoSize = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Tahoma'
    Font.Style = []
    MaxValue = 10000
    MinValue = -10000
    ParentFont = False
    TabOrder = 15
    OnChange = PorogChange
    OnKeyPress = RightKeyPress2
  end
  object checkChann5: TCheckBox
    Left = 506
    Top = 255
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '6'
    TabOrder = 16
  end
  object checkChann6: TCheckBox
    Left = 506
    Top = 270
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '7'
    TabOrder = 17
  end
  object checkChann7: TCheckBox
    Left = 506
    Top = 284
    Width = 31
    Height = 19
    Anchors = [akTop, akRight]
    Caption = '8'
    TabOrder = 18
  end
end
